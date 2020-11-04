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

#include <cstdint>
#include <mutex>
#include <utility>

namespace restinio
{

namespace connection_count_limits
{

namespace impl
{

//FIXME: document this!
class acceptor_callback_iface_t
{
public:
	virtual void
	call_accept_now( std::size_t index ) noexcept = 0;

	virtual void
	schedule_next_accept_attempt( std::size_t index ) noexcept = 0;
};

//FIXME: document this!
template< typename Mutex_Type >
class actual_limiter_t
{
	Mutex_Type m_lock;
	acceptor_callback_iface_t * m_acceptor;

	std::size_t m_active_accepts{ 0u };
	std::size_t m_connections{ 0u };

	const std::size_t m_max_active_connections;
	std::vector< std::size_t > m_pending_indexes;

	bool
	has_free_slots() const noexcept
	{
		return (m_active_accepts + m_connections) < m_max_active_connections;
	}

public:
	actual_limiter_t(
		acceptor_callback_iface_t * acceptor,
		std::size_t max_active_connections,
		std::size_t max_pending_indexes )
		:	m_acceptor{ acceptor }
		,	m_max_active_connections{ max_active_connections }
	{
		m_pending_indexes.reserve( max_pending_indexes );
	}

	actual_limiter_t( const actual_limiter_t & ) = delete;
	actual_limiter_t( actual_limiter_t && ) = delete;

	void
	increment_active_connections() noexcept
	{
		std::lock_guard< Mutex_Type > lock{ m_lock };
		--m_active_accepts;
		++m_connections;
	}

	// Note: this method is noexcept because it can be called from
	// destructors.
	void
	decrement_active_connections() noexcept
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

} /* namespace connection_count_limits */

//FIXME: document this!
template< typename Strand >
class noop_connection_count_limiter_t
{
	connection_count_limits::impl::acceptor_callback_iface_t * m_acceptor;

public:
	noop_connection_count_limiter_t(
		connection_count_limits::impl::acceptor_callback_iface_t * acceptor,
		std::size_t /*max_active_connections*/,
		std::size_t /*max_pending_indexes*/ )
		:	m_acceptor{ acceptor }
	{
	}

	void
	increment_active_connections() noexcept { }

	void
	decrement_active_connections() noexcept { }

	void
	accept_next( std::size_t index ) noexcept
	{
		m_acceptor->call_accept_now( index );
	}
};

template< typename Strand >
class connection_count_limiter_t;

//FIXME: noop_strand_t and default_strand_t should be defined in
//a separate header file.
template<>
class connection_count_limiter_t< noop_strand_t >
	:	public connection_count_limits::impl::actual_limiter_t< null_mutex_t >
{
	using base_t = connection_count_limits::impl::actual_limiter_t< null_mutex_t >;

public:
	using base_t::base_t;
};

template<>
class connection_count_limiter_t< default_strand_t >
	:	public connection_count_limits::impl::actual_limiter_t< std::mutex >
{
	using base_t = connection_count_limits::impl::actual_limiter_t< std::mutex >;

public:
	using base_t::base_t;
};

namespace connection_count_limits
{

//FIXME: document this!
template< typename Count_Manager >
class connection_lifetime_monitor_t
{
	Count_Manager * m_manager;

public:
	connection_lifetime_monitor_t(
		Count_Manager * manager ) noexcept
		:	m_manager{ manager }
	{
		m_manager->increment_active_connections();
	}

	~connection_lifetime_monitor_t()
	{
		if( m_manager )
			m_manager->decrement_active_connections();
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

	//FIXME: is it really needed?
	bool
	empty() const noexcept
	{
		return nullptr == m_manager;
	}
};

template< typename Strand >
class connection_lifetime_monitor_t< noop_connection_count_limiter_t<Strand> >
{
public:
	connection_lifetime_monitor_t(
		noop_connection_count_limiter_t<Strand> * ) noexcept
	{}
};

} /* namespace connection_count_limits */

//FIXME: document this!
template< typename Traits >
struct connection_count_limit_types
{
	using limiter_t = typename Traits::template connection_count_limiter_t<
			typename Traits::strand_t >;

	using lifetime_monitor_t =
			connection_count_limits::connection_lifetime_monitor_t< limiter_t >;
};

} /* namespace restinio */

