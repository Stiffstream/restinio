/*
 * RESTinio
 */

/*!
 * @file
 * @brief Stuff related to limits of active parallel connections.
 * @since v.0.6.12
 */

#pragma once

#include <restinio/null_mutex.hpp>
#include <restinio/default_strands.hpp>

#include <restinio/utils/tagged_scalar.hpp>

#include <cstdint>
#include <mutex>
#include <utility>

namespace restinio
{

namespace connection_count_limits
{

//
// max_parallel_connections_t
//
struct max_parallel_connections_tag {};

/*!
 * @brief A kind of strict typedef for maximum count of active connections.
 *
 * @since v.0.6.12
 */
using max_parallel_connections_t = restinio::utils::tagged_scalar_t<
		std::size_t, max_parallel_connections_tag >;

//
// max_active_accepts_t
//
struct max_active_accepts_tag {};

/*!
 * @brief A kind of strict typedef for maximum count of active accepts.
 *
 * @since v.0.6.12
 */
using max_active_accepts_t = restinio::utils::tagged_scalar_t<
		std::size_t, max_active_accepts_tag >;

namespace impl
{

/*!
 * @brief An interface of acceptor to be used by connection count limiters.
 *
 * An instance of a connection count limiter will receive a reference to the
 * acceptor. The limiter has to call the acceptor and this interface declares
 * methods of the acceptor that will be invoked by the limiter.
 *
 * The assumed working scheme is:
 *
 * - the acceptor calls `accept_next` for the limiter;
 * - the limiter checks the possibility to call `accept()`. If it is possible,
 *   then the limiter calls `call_accept_now` back (right inside `accept_next`
 *   invocation). If it isn't possible, then the limiter stores the socket's
 *   slot index somewhere inside the limiter;
 * - sometime later the limiter calls `schedule_next_accept_attempt` for the
 *   acceptor. The acceptor then should perform a new call to `accept_next` in
 *   the appropriate worker context.
 *
 * @since v.0.6.12
 */
class acceptor_callback_iface_t
{
public:
	/*!
	 * This method will be invoked by a limiter when there is a possibility to
	 * call accept() right now.
	 */
	virtual void
	call_accept_now(
		//! An index of socket's slot to be used for accept().
		std::size_t index ) noexcept = 0;

	/*!
	 * This method will be invoked by a limiter when there is no possibility to
	 * call accept() right now, but the next call to `accept_next` should be
	 * scheduled as soon as possible in the appropriate worker context.
	 * 
	 * It is assumed that the acceptor will use asio::post() with a completion
	 * handler that calls the `accept_next` method of the limiter.
	 */
	virtual void
	schedule_next_accept_attempt(
		//! An index of socket's slot to be used for accept().
		std::size_t index ) noexcept = 0;
};

/*!
 * @brief Actual implementation of connection count limiter.
 *
 * @note
 * This is not Copyable nor Moveable type.
 *
 * @tparam Mutex_Type Type of mutex to be used for protection of limiter
 * object. It is expected to be std::mutex or null_mutex_t.
 *
 * @since v.0.6.12
 */
template< typename Mutex_Type >
class actual_limiter_t
{
	//! Lock object to be used.
	Mutex_Type m_lock;

	//! Mandatory pointer to the acceptor connected with this limiter.
	not_null_pointer_t< acceptor_callback_iface_t > m_acceptor;

	/*!
	 * @brief The counter of active accept() operations.
	 *
	 * Incremented every time the acceptor_callback_iface_t::call_accept_now()
	 * is invoked. Decremented in increment_parallel_connections().
	 *
	 * @attention
	 * It seems to be a fragile scheme because if there won't be a call to
	 * increment_parallel_connections() after the invocation of
	 * acceptor_callback_iface_t::call_accept_now() the value of
	 * m_active_accepts will be incorrect. But it is hard to invent a more
	 * bulletproof solution and it seems that the missing call to
	 * increment_parallel_connections() could be only on the shutdown of the
	 * acceptor.
	 */
	std::size_t m_active_accepts{ 0u };

	/*!
	 * @brief The counter of active connections.
	 *
	 * This value is incremented in increment_parallel_connections()
	 * and decremented in decrement_parallel_connections().
	 */
	std::size_t m_connections{ 0u };

	//! The limit for parallel connections.
	const std::size_t m_max_parallel_connections;

	/*!
	 * @brief The storage for holding pending socket's slots.
	 *
	 * @note
	 * This storage is used as stack: new indexes are added to the
	 * end and are got from the end of the vector (LIFO working scheme).
	 *
	 * @attention
	 * The capacity for that storage is preallocated in the constructor
	 * so we don't expect any allocations during the usage of
	 * m_pending_indexes. This allows accept_next() method to be
	 * noexcept. But this works only if max_pending_indexes passed
	 * to the constructor is right.
	 */
	std::vector< std::size_t > m_pending_indexes;

	RESTINIO_NODISCARD
	bool
	has_free_slots() const noexcept
	{
		return (m_active_accepts + m_connections) < m_max_parallel_connections;
	}

public:
	actual_limiter_t(
		not_null_pointer_t< acceptor_callback_iface_t > acceptor,
		max_parallel_connections_t max_parallel_connections,
		max_active_accepts_t max_pending_indexes )
		:	m_acceptor{ acceptor }
		,	m_max_parallel_connections{ max_parallel_connections.value() }
	{
		m_pending_indexes.reserve( max_pending_indexes.value() );
	}

	actual_limiter_t( const actual_limiter_t & ) = delete;
	actual_limiter_t( actual_limiter_t && ) = delete;

	void
	increment_parallel_connections() noexcept
	{
		std::lock_guard< Mutex_Type > lock{ m_lock };

		// Expects that m_active_accepts is always greater than 0.
		--m_active_accepts;

		++m_connections;
	}

	// Note: this method is noexcept because it can be called from
	// destructors.
	void
	decrement_parallel_connections() noexcept
	{
		// Decrement active connections under acquired lock.
		// If the count of connections drops below the limit and
		// there are some pending indexes then one of them will
		// be returned (wrapped into an optional).
		auto index_to_activate = [this]() -> optional_t<std::size_t> {
			std::lock_guard< Mutex_Type > lock{ m_lock };

			// Expects that m_connections is always greater than 0.
			--m_connections;

			if( has_free_slots() && !m_pending_indexes.empty() )
			{
				std::size_t pending_index = m_pending_indexes.back();
				m_pending_indexes.pop_back();
				return pending_index;
			}
			else
				return nullopt;
		}();

		if( index_to_activate )
		{
			m_acceptor->schedule_next_accept_attempt( *index_to_activate );
		}
	}

	/*!
	 * This method either calls acceptor_callback_iface_t::call_accept_now() (in
	 * that case m_active_accepts is incremented) or stores @a index into the
	 * internal storage.
	 */
	void
	accept_next( std::size_t index ) noexcept
	{
		// Perform all operations under acquired lock.
		// The result is a flag that tells can accept() be called right now.
		const bool accept_now = [this, index]() -> bool {
			std::lock_guard< Mutex_Type > lock{ m_lock };

			if( has_free_slots() )
			{
				++m_active_accepts;
				return true;
			}
			else
			{
				m_pending_indexes.push_back( index );
				return false;
			}
		}();

		if( accept_now )
		{
			m_acceptor->call_accept_now( index );
		}
	}
};

} /* namespace impl */

/*!
 * @brief An implementation of connection count limiter for the case
 * when connection count is not limited.
 *
 * @since v.0.6.12
 */
class noop_connection_count_limiter_t
{
	not_null_pointer_t< connection_count_limits::impl::acceptor_callback_iface_t > m_acceptor;

public:
	noop_connection_count_limiter_t(
		not_null_pointer_t< connection_count_limits::impl::acceptor_callback_iface_t > acceptor,
		max_parallel_connections_t /*max_parallel_connections*/,
		max_active_accepts_t /*max_pending_indexes*/ )
		:	m_acceptor{ acceptor }
	{
	}

	void
	increment_parallel_connections() noexcept { /* Nothing to do */ }

	void
	decrement_parallel_connections() noexcept { /* Nothing to do */ }

	/*!
	 * Calls acceptor_callback_iface_t::call_accept_now() directly.
	 * The @a index is never stored anywhere.
	 */
	void
	accept_next( std::size_t index ) noexcept
	{
		m_acceptor->call_accept_now( index );
	}
};

/*!
 * @brief Template class for connection count limiter for the case when
 * connection count limit is actually used.
 *
 * The actual implementation will be provided by specializations of
 * that class for specific Strand types.
 *
 * @since v.0.6.12
 */
template< typename Strand >
class connection_count_limiter_t;

/*!
 * @brief Implementation of connection count limiter for single-threading
 * mode.
 *
 * In single-threading mode there is no need to protect limiter from
 * access from different threads. So null_mutex_t is used.
 *
 * @since v.0.6.12
 */
template<>
class connection_count_limiter_t< noop_strand_t >
	:	public connection_count_limits::impl::actual_limiter_t< null_mutex_t >
{
	using base_t = connection_count_limits::impl::actual_limiter_t< null_mutex_t >;

public:
	using base_t::base_t;
};

/*!
 * @brief Implementation of connection count limiter for multi-threading
 * mode.
 *
 * In multi-threading mode std::mutex is used for the protection of
 * limiter object.
 *
 * @since v.0.6.12
 */
template<>
class connection_count_limiter_t< default_strand_t >
	:	public connection_count_limits::impl::actual_limiter_t< std::mutex >
{
	using base_t = connection_count_limits::impl::actual_limiter_t< std::mutex >;

public:
	using base_t::base_t;
};

/*!
 * @brief Helper type for controlling the lifetime of the connection.
 *
 * Connection count limiter should be informed when a new connection
 * created and when an existing connection is closed. An instance
 * of connection_lifetime_monitor_t should be used for that purpose:
 * a new instance of connection_lifetime_monitor_t should be created
 * and bound to a connection object. The constructor of
 * connection_lifetime_monitor_t will inform the limiter about
 * the creation of a new connection. The destructor of
 * connection_lifetime_monitor_t will inform the limiter about the
 * destruction of a connection.
 *
 * @note
 * This type is not Copyable but Movabale.
 *
 * @attention
 * The pointer to Count_Manager passed to the constructor should
 * remain valid the whole lifetime of connection_lifetime_monitor_t
 * instance.
 *
 * @since v.0.6.12
 */
template< typename Count_Manager >
class connection_lifetime_monitor_t
{
	not_null_pointer_t< Count_Manager > m_manager;

public:
	connection_lifetime_monitor_t(
		not_null_pointer_t< Count_Manager > manager ) noexcept
		:	m_manager{ manager }
	{
		m_manager->increment_parallel_connections();
	}

	~connection_lifetime_monitor_t()
	{
		if( m_manager )
			m_manager->decrement_parallel_connections();
	}

	connection_lifetime_monitor_t(
		const connection_lifetime_monitor_t & ) = delete;

	friend void
	swap(
		connection_lifetime_monitor_t & a,
		connection_lifetime_monitor_t & b ) noexcept
	{
		using std::swap;
		swap( a.m_manager, b.m_manager );
	}

	connection_lifetime_monitor_t(
		connection_lifetime_monitor_t && other ) noexcept
		:	m_manager{ other.m_manager }
	{
		other.m_manager = nullptr;
	}

	connection_lifetime_monitor_t &
	operator=( connection_lifetime_monitor_t && other ) noexcept
	{
		connection_lifetime_monitor_t tmp{ std::move(other) };
		swap( *this, tmp );
		return *this;
	}

	connection_lifetime_monitor_t &
	operator=( const connection_lifetime_monitor_t & ) = delete;
};

/*!
 * @brief Specialization of connection_lifetime_monitor for the case
 * when connection count limiter is not used at all.
 *
 * Holds nothing. Does nothing.
 *
 * @since v.0.6.12
 */
template<>
class connection_lifetime_monitor_t< noop_connection_count_limiter_t >
{
public:
	connection_lifetime_monitor_t(
		not_null_pointer_t< noop_connection_count_limiter_t > ) noexcept
	{}
};

} /* namespace connection_count_limits */

/*!
 * @brief A kind of metafunction that deduces actual types related
 * to connection count limiter in the dependecy of Traits.
 *
 * Deduces the following types:
 *
 * - limiter_t. The actual type of connection count limiter to be
 *   used in the RESTinio's server;
 * - lifetime_monitor_t. The actual type of connection_lifetime_monitor
 *   to be used with connection objects.
 *
 * @tparam Traits The type with traits for RESTinio's server.
 *
 * @since v.0.6.12
 */
template< typename Traits >
struct connection_count_limit_types
{
	using limiter_t = typename std::conditional
		<
			Traits::use_connection_count_limiter,
			connection_count_limits::connection_count_limiter_t<
					typename Traits::strand_t >,
			connection_count_limits::noop_connection_count_limiter_t
		>::type;

	using lifetime_monitor_t =
			connection_count_limits::connection_lifetime_monitor_t< limiter_t >;
};

} /* namespace restinio */

