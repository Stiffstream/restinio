/*
	restinio
*/

/*!
	HTTP-Server configuration.
*/

#pragma once

#include <chrono>
#include <tuple>
#include <utility>

#include <restinio/asio_include.hpp>

#include <restinio/exception.hpp>
#include <restinio/request_handler.hpp>
#include <restinio/traits.hpp>

namespace restinio
{

namespace details
{

//! Default instantiation for a specific type.
template < typename Object >
inline auto
create_default_unique_object_instance( std::false_type )
{
	return std::unique_ptr< Object >{};
}

template < typename Object >
inline auto
create_default_unique_object_instance( std::true_type )
{
	return std::make_unique< Object >();
}

//! Default instantiation for a specific type.
template < typename Object >
inline auto
create_default_shared_object_instance( std::false_type )
{
	return std::shared_ptr< Object >{};
}

template < typename Object >
inline auto
create_default_shared_object_instance( std::true_type )
{
	return std::make_shared< Object >();
}

} /* namespace details */

//
// create_default_unique_object_instance
//

//! Default instantiation for a specific type.
template < typename Object>
inline auto
create_default_unique_object_instance()
{
	typename std::is_default_constructible< Object >::type tag;
	return details::create_default_unique_object_instance< Object >( tag );
}

//! Default instantiation for default_request_handler_t.
template <>
inline auto
create_default_unique_object_instance< default_request_handler_t >()
{
	return details::create_default_unique_object_instance< default_request_handler_t >(
			std::false_type{} );
}

//
// create_default_shared_object_instance
//

//! Default instantiation for a specific type.
template < typename Object>
inline auto
create_default_shared_object_instance()
{
	typename std::is_default_constructible< Object >::type tag;
	return details::create_default_shared_object_instance< Object >( tag );
}

//! Default instantiation for default_request_handler_t.
template <>
inline auto
create_default_shared_object_instance< default_request_handler_t >()
{
	return details::create_default_shared_object_instance< default_request_handler_t >(
			std::false_type{} );
}

//
// ensure_created()
//

//! Ensure that object was created.
template < typename Object >
auto
ensure_created(
	std::unique_ptr< Object > mb_created_one,
	string_view_t fail_description )
{
	if( !mb_created_one )
		mb_created_one = create_default_unique_object_instance< Object >();

	if( !mb_created_one )
		throw exception_t{ fail_description };

	return mb_created_one;
}

//
// unsure_created()
//

//! Ensure that object was created.
template < typename Object >
auto
ensure_created(
	std::shared_ptr< Object > mb_created_one,
	string_view_t fail_description )
{
	if( !mb_created_one )
		mb_created_one = create_default_shared_object_instance< Object >();

	if( !mb_created_one )
		throw exception_t{ fail_description };

	return mb_created_one;
}


//
// socket_type_dependent_settings_t
//

//! Extra settings needed for working with socket.
template < typename Settings, typename Socket >
class socket_type_dependent_settings_t
{
protected :
	~socket_type_dependent_settings_t() noexcept = default;

public :
	socket_type_dependent_settings_t() noexcept = default;

	socket_type_dependent_settings_t(const socket_type_dependent_settings_t &) noexcept = default;
	socket_type_dependent_settings_t(socket_type_dependent_settings_t &&) noexcept = default;

	socket_type_dependent_settings_t &
	operator=(const socket_type_dependent_settings_t &) noexcept = default;

	socket_type_dependent_settings_t &
	operator=(socket_type_dependent_settings_t &&) noexcept = delete;

	// No extra settings by default.
};

//
// acceptor_options_t
//

//! An adapter for setting acceptor options before running server.
/*!
	Class hides an acceptor object and opens only set/get options API.
	It is used as an argument for a user defined function-object
	that can set custom options for acceptor.
*/
class acceptor_options_t
{
	public:
		acceptor_options_t( asio_ns::ip::tcp::acceptor & acceptor )
			:	m_acceptor{ acceptor }
		{}

		//! API for setting/getting options.
		//! \{
		template< typename Option >
		void
		set_option( const Option & option )
		{
			m_acceptor.set_option( option );
		}

		template< typename Option >
		void
		set_option( const Option & option, asio_ns::error_code & ec )
		{
			m_acceptor.set_option( option, ec );
		}

		template< typename Option >
		void
		get_option( Option & option )
		{
			m_acceptor.get_option( option );
		}

		template< typename Option >
		void
		get_option( Option & option, asio_ns::error_code & ec )
		{
			m_acceptor.get_option( option, ec );
		}
		//! \}

	private:
		asio_ns::ip::tcp::acceptor & m_acceptor;
};

using acceptor_options_setter_t = std::function< void ( acceptor_options_t & ) >;

template <>
inline auto
create_default_unique_object_instance< acceptor_options_setter_t >()
{
	return std::make_unique< acceptor_options_setter_t >(
		[]( acceptor_options_t & options ){
			options.set_option( asio_ns::ip::tcp::acceptor::reuse_address( true ) );
		} );
}

//
// socket_options_t
//

//! An adapter for setting acceptor options before running server.
/*!
	Class hides a socket object and opens only set/get options API.
	It is used as an argument for a user defined function-object
	that can set custom options for socket.
*/
class socket_options_t
{
	public:
		socket_options_t(
			//! A reference on the most base class with interface of setting options.
			asio_ns::basic_socket< asio_ns::ip::tcp > & socket )
			:	m_socket{ socket }
		{}

		//! API for setting/getting options.
		//! \{
		template< typename Option >
		void
		set_option( const Option & option )
		{
			m_socket.set_option( option );
		}

		template< typename Option >
		void
		set_option( const Option & option, asio_ns::error_code & ec )
		{
			m_socket.set_option( option, ec );
		}

		template< typename Option >
		void
		get_option( Option & option )
		{
			m_socket.get_option( option );
		}

		template< typename Option >
		void
		get_option( Option & option, asio_ns::error_code & ec )
		{
			m_socket.get_option( option, ec );
		}
		//! \}

	private:
		//! A reference on the most base class with interface of setting options.
		asio_ns::basic_socket< asio_ns::ip::tcp > & m_socket;
};

using socket_options_setter_t = std::function< void ( socket_options_t & ) >;

template <>
inline auto
create_default_unique_object_instance< socket_options_setter_t >()
{
	return std::make_unique< socket_options_setter_t >( []( auto & ){} );
}

//
// cleanup_functor_t
//
/*!
 * \brief Type of holder for user's cleanup function.
 */
using cleanup_functor_t = std::function< void(void) >;

//
// connection_state_listener_holder_t
//
/*!
 * @brief A special class for holding actual connection state listener.
 *
 * This class holds shared pointer to actual connection state listener
 * and provides an actual implementation of
 * check_valid_connection_state_listener_pointer() method.
 *
 * @since v.0.5.1
 */
template< typename Listener >
struct connection_state_listener_holder_t
{
	std::shared_ptr< Listener > m_connection_state_listener;

	static constexpr bool has_actual_connection_state_listener = true;

	//! Checks that pointer to state listener is not null.
	/*!
	 * Throws an exception if m_connection_state_listener is nullptr.
	 */
	void
	check_valid_connection_state_listener_pointer() const
	{
		if( !m_connection_state_listener )
			throw exception_t{ "connection state listener is not specified" };
	}
};

/*!
 * @brief A special class for case when no-op state listener is used.
 *
 * Doesn't hold anything and contains empty
 * check_valid_connection_state_listener_pointer() method.
 *
 * @since v.0.5.1
 */
template<>
struct connection_state_listener_holder_t< connection_state::noop_listener_t >
{
	static constexpr bool has_actual_connection_state_listener = false;

	void
	check_valid_connection_state_listener_pointer() const
	{
		// Nothing to do.
	}
};

//
// ip_blocker_holder_t
//
/*!
 * @brief A special class for holding actual IP-blocker object.
 *
 * This class holds shared pointer to actual IP-blocker
 * and provides an actual implementation of
 * check_valid_ip_blocker_pointer() method.
 *
 * @since v.0.5.1
 */
template< typename Ip_Blocker >
struct ip_blocker_holder_t
{
	static_assert(
			noexcept( std::declval<Ip_Blocker>().inspect(
					std::declval<ip_blocker::incoming_info_t>() ) ),
			"Ip_Blocker::inspect() method should be noexcept" );

	static_assert(
			std::is_same<
					restinio::ip_blocker::inspection_result_t,
					decltype(std::declval<Ip_Blocker>().inspect(
							std::declval<ip_blocker::incoming_info_t>())) >::value,
			"Ip_Blocker::inspect() should return "
			"restinio::ip_blocker::inspection_result_t" );

	std::shared_ptr< Ip_Blocker > m_ip_blocker;

	static constexpr bool has_actual_ip_blocker = true;

	//! Checks that pointer to IP-blocker is not null.
	/*!
	 * Throws an exception if m_ip_blocker is nullptr.
	 */
	void
	check_valid_ip_blocker_pointer() const
	{
		if( !m_ip_blocker )
			throw exception_t{ "IP-blocker is not specified" };
	}
};

/*!
 * @brief A special class for case when no-op IP-blocker is used.
 *
 * Doesn't hold anything and contains empty
 * check_valid_ip_blocker_pointer() method.
 *
 * @since v.0.5.1
 */
template<>
struct ip_blocker_holder_t< ip_blocker::noop_ip_blocker_t >
{
	static constexpr bool has_actual_ip_blocker = false;

	void
	check_valid_ip_blocker_pointer() const
	{
		// Nothing to do.
	}
};

//
// basic_server_settings_t
//

//! Basic container for http_server settings.
/*!
 * It exists to provide ablity to create various derived classes
 * like server_settings_t, run_on_this_thread_settings_t,
 * run_on_this_thread_settings_t and so on.
 *
 * \tparam Derived A drived type. Reference to this derived type
 * will be returned by setters.
 *
 * \tparam Traits A type with traits for http_server.
 */
template<typename Derived, typename Traits>
class basic_server_settings_t
	:	public socket_type_dependent_settings_t< Derived, typename Traits::stream_socket_t >
	,	protected connection_state_listener_holder_t<
			typename Traits::connection_state_listener_t >
	,	protected ip_blocker_holder_t< typename Traits::ip_blocker_t >
{
		using base_type_t = socket_type_dependent_settings_t<
				Derived, typename Traits::stream_socket_t>;

		using connection_state_listener_holder_t<
						typename Traits::connection_state_listener_t
					>::has_actual_connection_state_listener;

		using ip_blocker_holder_t<
						typename Traits::ip_blocker_t
					>::has_actual_ip_blocker;

	public:
		basic_server_settings_t(
			std::uint16_t port = 8080,
			asio_ns::ip::tcp protocol = asio_ns::ip::tcp::v4() )
			:	base_type_t{}
			,	m_port{ port }
			,	m_protocol{ protocol }
		{}

		//! Server endpoint.
		//! \{
		Derived &
		port( std::uint16_t p ) &
		{
			m_port = p;
			return reference_to_derived();
		}

		Derived &&
		port( std::uint16_t p ) &&
		{
			return std::move( this->port( p ) );
		}

		std::uint16_t
		port() const
		{
			return m_port;
		}

		Derived &
		protocol( asio_ns::ip::tcp p ) &
		{
			m_protocol = p;
			return reference_to_derived();
		}

		Derived &&
		protocol( asio_ns::ip::tcp p ) &&
		{
			return std::move( this->protocol( p ) );
		}

		asio_ns::ip::tcp
		protocol() const
		{
			return m_protocol;
		}

		Derived &
		address( std::string addr ) &
		{
			m_address = std::move(addr);
			return reference_to_derived();
		}

		Derived &&
		address( std::string addr ) &&
		{
			return std::move( this->address( std::move( addr ) ) );
		}

		const std::string &
		address() const
		{
			return m_address;
		}
		//! \}

		//! Size of buffer for io operations.
		/*!
			It limits a size of chunk that can be read from socket in a single
			read operattion (async read).
		*/
		//! {
		Derived &
		buffer_size( std::size_t s ) &
		{
			m_buffer_size = s;
			return reference_to_derived();
		}

		Derived &&
		buffer_size( std::size_t s ) &&
		{
			return std::move( this->buffer_size( s ) );
		}

		std::size_t
		buffer_size() const
		{
			return m_buffer_size;
		}
		//! }

		//! A period for holding connection before completely receiving
		//! new http-request. Starts counting since connection is establised
		//! or a previous request was responsed.
		/*!
			Generaly it defines timeout for keep-alive connections.
		*/
		//! \{
		Derived &
		read_next_http_message_timelimit( std::chrono::steady_clock::duration d ) &
		{
			m_read_next_http_message_timelimit = std::move( d );
			return reference_to_derived();
		}

		Derived &&
		read_next_http_message_timelimit( std::chrono::steady_clock::duration d ) &&
		{
			return std::move( this->read_next_http_message_timelimit( std::move( d ) ) );
		}

		std::chrono::steady_clock::duration
		read_next_http_message_timelimit() const
		{
			return m_read_next_http_message_timelimit;
		}
		//! \}

		//! A period of time wait for response to be written to socket.
		//! \{
		Derived &
		write_http_response_timelimit( std::chrono::steady_clock::duration d ) &
		{
			m_write_http_response_timelimit = std::move( d );
			return reference_to_derived();
		}

		Derived &&
		write_http_response_timelimit( std::chrono::steady_clock::duration d ) &&
		{
			return std::move( this->write_http_response_timelimit( std::move( d ) ) );
		}

		std::chrono::steady_clock::duration
		write_http_response_timelimit() const
		{
			return m_write_http_response_timelimit;
		}
		//! \}

		//! A period of time that is given for a handler to create response.
		//! \{
		Derived &
		handle_request_timeout( std::chrono::steady_clock::duration d ) &
		{
			m_handle_request_timeout = std::move( d );
			return reference_to_derived();
		}

		Derived &&
		handle_request_timeout( std::chrono::steady_clock::duration d ) &&
		{
			return std::move( this->handle_request_timeout( std::move( d ) ) );
		}

		std::chrono::steady_clock::duration
		handle_request_timeout() const
		{
			return m_handle_request_timeout;
		}
		//! \}

		//! Max pipelined requests able to receive on single connection.
		//! \{
		Derived &
		max_pipelined_requests( std::size_t mpr ) &
		{
			m_max_pipelined_requests = mpr;
			return reference_to_derived();
		}

		Derived &&
		max_pipelined_requests( std::size_t mpr ) &&
		{
			return std::move( this->max_pipelined_requests( mpr ) );
		}

		std::size_t
		max_pipelined_requests() const
		{
			return m_max_pipelined_requests;
		}
		//! \}


		//! Request handler.
		//! \{
		using request_handler_t = typename Traits::request_handler_t;

		Derived &
		request_handler( std::unique_ptr< request_handler_t > handler ) &
		{
			m_request_handler = std::move( handler );
			return reference_to_derived();
		}

		template< typename... Params >
		Derived &
		request_handler( Params &&... params ) &
		{
			return set_unique_instance(
					m_request_handler,
					std::forward< Params >( params )... );
		}


		template< typename... Params >
		Derived &&
		request_handler( Params &&... params ) &&
		{
			return std::move( this->request_handler( std::forward< Params >( params )... ) );
		}

		std::unique_ptr< request_handler_t >
		request_handler()
		{
			return ensure_created(
				std::move( m_request_handler ),
				"request handler must be set" );
		}
		//! \}


		//! Timers manager.
		//! \{
		using timer_manager_t = typename Traits::timer_manager_t;
		using timer_factory_t = typename timer_manager_t::factory_t;

		template< typename... Params >
		Derived &
		timer_manager( Params &&... params ) &
		{
			return set_unique_instance(
					m_timer_factory,
					std::forward< Params >( params )... );
		}

		template< typename... Params >
		Derived &&
		timer_manager( Params &&... params ) &&
		{
			return std::move( this->timer_manager( std::forward< Params >( params )... ) );
		}

		std::unique_ptr< timer_factory_t >
		timer_factory()
		{
			return ensure_created(
				std::move( m_timer_factory ),
				"timer manager is not set" );
		}
		//! \}

		//! Logger.
		//! \{
		using logger_t = typename Traits::logger_t;

		template< typename... Params >
		Derived &
		logger( Params &&... params ) &
		{
			return set_unique_instance(
					m_logger,
					std::forward< Params >( params )... );
		}

		template< typename... Params >
		Derived &&
		logger( Params &&... params ) &&
		{
			return std::move( this->logger( std::forward< Params >( params )... ) );
		}

		std::unique_ptr< logger_t >
		logger()
		{
			return ensure_created(
				std::move( m_logger ),
				"logger must be set" );
		}
		//! \}

		//! Acceptor options setter.
		//! \{
		Derived &
		acceptor_options_setter( acceptor_options_setter_t aos ) &
		{
			if( !aos )
				throw exception_t{ "acceptor options setter cannot be empty" };

			return set_unique_instance(
					m_acceptor_options_setter,
					std::move( aos ) );
		}

		Derived &&
		acceptor_options_setter( acceptor_options_setter_t aos ) &&
		{
			return std::move( this->acceptor_options_setter( std::move( aos ) ) );
		}

		std::unique_ptr< acceptor_options_setter_t >
		acceptor_options_setter()
		{
			return ensure_created(
				std::move( m_acceptor_options_setter ),
				"acceptor options setter must be set" );
		}
		//! \}

		//! Socket options setter.
		//! \{
		Derived &
		socket_options_setter( socket_options_setter_t sos ) &
		{
			if( !sos )
				throw exception_t{ "socket options setter cannot be empty" };

			return set_unique_instance(
					m_socket_options_setter,
					std::move( sos ) );
		}

		Derived &&
		socket_options_setter( socket_options_setter_t sos ) &&
		{
			return std::move( this->socket_options_setter( std::move( sos ) ) );
		}

		std::unique_ptr< socket_options_setter_t >
		socket_options_setter()
		{
			return ensure_created(
				std::move( m_socket_options_setter ),
				"socket options setter must be set" );
		}
		//! \}

		//! Max number of running concurrent accepts.
		/*!
			When running server on N threads
			then up to N accepts can be handled concurrently.
		*/
		//! \{
		Derived &
		concurrent_accepts_count( std::size_t n ) &
		{
			if( 0 == n || 1024 < n )
				throw exception_t{
					fmt::format(
						"invalid value for number of cuncurrent connects: {}",
						n ) };

			m_concurrent_accepts_count = n;
			return reference_to_derived();
		}

		Derived &&
		concurrent_accepts_count( std::size_t n ) &&
		{
			return std::move( this->concurrent_accepts_count( n ) );
		}

		std::size_t
		concurrent_accepts_count() const
		{
			return m_concurrent_accepts_count;
		}
		//! \}

		//! Do separate an accept operation and connection instantiation.
		/*!
			For the cases when a lot of connection can be fired by clients
			in a short time interval, it is vital to accept connections
			and initiate new accept operations as quick as possible.
			So creating connection instance that involves allocations
			and initialization can be done in a context that
			is independent to acceptors one.
		*/
		//! \{
		Derived &
		separate_accept_and_create_connect( bool do_separate ) &
		{
			m_separate_accept_and_create_connect = do_separate;
			return reference_to_derived();
		}

		Derived &&
		separate_accept_and_create_connect( bool do_separate ) &&
		{
			return std::move( this->separate_accept_and_create_connect( do_separate ) );
		}

		bool
		separate_accept_and_create_connect() const
		{
			return m_separate_accept_and_create_connect;
		}
		//! \}

		//! Cleanup function.
		//! \{
		template< typename Func >
		Derived &
		cleanup_func( Func && func ) &
		{
			m_cleanup_functor = std::move(func);
			return reference_to_derived();
		}

		template< typename Func >
		Derived &&
		cleanup_func( Func && func ) &&
		{
			return std::move(this->cleanup_func( std::forward<Func>(func) ));
		}

		cleanup_functor_t
		giveaway_cleanup_func()
		{
			return std::move(m_cleanup_functor);
		}
		//! \}

		/*!
		 * @brief Setter for connection state listener.
		 *
		 * @note connection_state_listener() method should be called if
		 * user specify its type for connection_state_listener_t traits.
		 * For example:
		 * @code
		 * class my_state_listener_t {
		 * 	...
		 * public:
		 * 	...
		 * 	void state_changed(const restinio::connection_state::notice_t & notice) noexcept {
		 * 		...
		 * 	}
		 * };
		 *
		 * struct my_traits_t : public restinio::default_traits_t {
		 * 	using connection_state_listener_t = my_state_listener_t;
		 * };
		 *
		 * restinio::server_setting_t<my_traits_t> settings;
		 * setting.connection_state_listener( std::make_shared<my_state_listener_t>(...) );
		 * ...
		 * @endcode
		 *
		 * @attention This method can't be called if the default no-op
		 * state listener is used in server traits.
		 *
		 * @since v.0.5.1
		 */
		Derived &
		connection_state_listener(
			std::shared_ptr< typename Traits::connection_state_listener_t > listener ) &
		{
			static_assert(
					has_actual_connection_state_listener,
					"connection_state_listener(listener) can't be used "
					"for the default connection_state::noop_listener_t" );

			this->m_connection_state_listener = std::move(listener);
			return reference_to_derived();
		}

		/*!
		 * @brief Setter for connection state listener.
		 *
		 * @note connection_state_listener() method should be called if
		 * user specify its type for connection_state_listener_t traits.
		 * For example:
		 * @code
		 * class my_state_listener_t {
		 * 	...
		 * public:
		 * 	...
		 * 	void state_changed(const restinio::connection_state::notice_t & notice) noexcept {
		 * 		...
		 * 	}
		 * };
		 *
		 * struct my_traits_t : public restinio::default_traits_t {
		 * 	using connection_state_listener_t = my_state_listener_t;
		 * };
		 *
		 * restinio::run( restinio::on_this_thread<my_traits_t>()
		 * 		.connection_state_listener( std::make_shared<my_state_listener_t>(...) )
		 * 		.port(...)
		 * 		...);
		 * @endcode
		 *
		 * @attention This method can't be called if the default no-op
		 * state listener is used in server traits.
		 *
		 * @since v.0.5.1
		 */
		Derived &&
		connection_state_listener(
			std::shared_ptr< typename Traits::connection_state_listener_t > listener ) &&
		{
			return std::move(this->connection_state_listener(
					std::move(listener)));
		}

		/*!
		 * @brief Get reference to connection state listener.
		 *
		 * @attention This method can't be called if the default no-op
		 * state listener is used in server traits.
		 *
		 * @since v.0.5.1
		 */
		const std::shared_ptr< typename Traits::connection_state_listener_t > &
		connection_state_listener() const noexcept
		{
			static_assert(
					has_actual_connection_state_listener,
					"connection_state_listener() can't be used "
					"for the default connection_state::noop_listener_t" );

			return this->m_connection_state_listener;
		}

		/*!
		 * @brief Internal method for checking presence of state listener
		 * object.
		 *
		 * If a user specifies custom state listener type but doesn't
		 * set a pointer to listener object that method throws an exception.
		 *
		 * @since v.0.5.1
		 */
		void
		ensure_valid_connection_state_listener()
		{
			this->check_valid_connection_state_listener_pointer();
		}

		/*!
		 * @brief Setter for IP-blocker.
		 *
		 * @note ip_blocker() method should be called if
		 * user specify its type for ip_blocker_t traits.
		 * For example:
		 * @code
		 * class my_ip_blocker_t {
		 * 	...
		 * public:
		 * 	...
		 * 	restinio::ip_blocker::inspection_result_t
		 * 	inspect(const restinio::ip_blocker::incoming_info_t & info) noexcept {
		 * 		...
		 * 	}
		 * };
		 *
		 * struct my_traits_t : public restinio::default_traits_t {
		 * 	using ip_blocker_t = my_ip_blocker_t;
		 * };
		 *
		 * restinio::server_setting_t<my_traits_t> settings;
		 * setting.ip_blocker( std::make_shared<my_ip_blocker_t>(...) );
		 * ...
		 * @endcode
		 *
		 * @attention This method can't be called if the default no-op
		 * IP-blocker is used in server traits.
		 *
		 * @since v.0.5.1
		 */
		Derived &
		ip_blocker(
			std::shared_ptr< typename Traits::ip_blocker_t > blocker ) &
		{
			static_assert(
					basic_server_settings_t::has_actual_ip_blocker,
					"ip_blocker(blocker) can't be used "
					"for the default ip_blocker::noop_ip_blocker_t" );

			this->m_ip_blocker = std::move(blocker);
			return reference_to_derived();
		}

		/*!
		 * @brief Setter for IP-blocker.
		 *
		 * @note ip_blocker() method should be called if
		 * user specify its type for ip_blocker_t traits.
		 * For example:
		 * @code
		 * class my_ip_blocker_t {
		 * 	...
		 * public:
		 * 	...
		 * 	restinio::ip_blocker::inspection_result_t
		 * 	inspect(const restinio::ip_blocker::incoming_info_t & info) noexcept {
		 * 		...
		 * 	}
		 * };
		 *
		 * struct my_traits_t : public restinio::default_traits_t {
		 * 	using ip_blocker_t = my_ip_blocker_t;
		 * };
		 *
		 * restinio::run( restinio::on_this_thread<my_traits_t>()
		 * 		.ip_blocker( std::make_shared<my_ip_blocker_t>(...) )
		 * 		.port(...)
		 * 		...);
		 * @endcode
		 *
		 * @attention This method can't be called if the default no-op
		 * state listener is used in server traits.
		 *
		 * @since v.0.5.1
		 */
		Derived &&
		ip_blocker(
			std::shared_ptr< typename Traits::ip_blocker_t > blocker ) &&
		{
			return std::move(this->ip_blocker(std::move(blocker)));
		}

		/*!
		 * @brief Get reference to IP-blocker.
		 *
		 * @attention This method can't be called if the default no-op
		 * IP-blocker is used in server traits.
		 *
		 * @since v.0.5.1
		 */
		const std::shared_ptr< typename Traits::ip_blocker_t > &
		ip_blocker() const noexcept
		{
			static_assert(
					basic_server_settings_t::has_actual_ip_blocker,
					"ip_blocker() can't be used "
					"for the default ip_blocker::noop_ip_blocker_t" );

			return this->m_ip_blocker;
		}

		/*!
		 * @brief Internal method for checking presence of IP-blocker object.
		 *
		 * If a user specifies custom IP-blocker type but doesn't
		 * set a pointer to blocker object that method throws an exception.
		 *
		 * @since v.0.5.1
		 */
		void
		ensure_valid_ip_blocker()
		{
			this->check_valid_ip_blocker_pointer();
		}

	private:
		Derived &
		reference_to_derived()
		{
			return static_cast<Derived &>(*this);
		}

		template< typename Target, typename... Params >
		Derived &
		set_unique_instance( std::unique_ptr< Target > & t, Params &&... params )
		{
			t =
				std::make_unique< Target >(
					std::forward< Params >( params )... );

			return reference_to_derived();
		}

		template< typename Target, typename... Params >
		Derived &
		set_shared_instance( std::shared_ptr< Target > & t, Params &&... params )
		{
			t =
				std::make_shared< Target >(
					std::forward< Params >( params )... );

			return reference_to_derived();
		}

		//! Server endpoint.
		//! \{
		std::uint16_t m_port;
		asio_ns::ip::tcp m_protocol;
		std::string m_address;
		//! \}

		//! Size of buffer for io operations.
		std::size_t m_buffer_size{ 4 * 1024 };

		//! Operations timeouts.
		//! \{
		std::chrono::steady_clock::duration
			m_read_next_http_message_timelimit{ std::chrono::seconds( 60 ) };

		std::chrono::steady_clock::duration
			m_write_http_response_timelimit{ std::chrono::seconds( 5 ) };

		std::chrono::steady_clock::duration
			m_handle_request_timeout{ std::chrono::seconds( 10 ) };
		//! \}

		//! Max pipelined requests to receive on single connection.
		std::size_t m_max_pipelined_requests{ 1 };

		//! Request handler.
		std::unique_ptr< request_handler_t > m_request_handler;

		//! Timers factory.
		std::unique_ptr< timer_factory_t > m_timer_factory;

		//! Logger.
		std::unique_ptr< logger_t > m_logger;

		//! Acceptor options setter.
		std::unique_ptr< acceptor_options_setter_t > m_acceptor_options_setter;

		//! Socket options setter.
		std::unique_ptr< socket_options_setter_t > m_socket_options_setter;

		std::size_t m_concurrent_accepts_count{ 1 };

		//! Do separate an accept operation and connection instantiation.
		bool m_separate_accept_and_create_connect{ false };

		//! Optional cleanup functor.
		cleanup_functor_t m_cleanup_functor;
};

//
// server_settings_t
//

//! A fluent style interface for setting http server params.
template<typename Traits = default_traits_t>
class server_settings_t final
	:	public basic_server_settings_t< server_settings_t<Traits>, Traits >
{
	using base_type_t = basic_server_settings_t<
				server_settings_t<Traits>, Traits>;
public:
	using base_type_t::base_type_t;
};

template < typename Traits, typename Configurator >
auto
exec_configurator( Configurator && configurator )
{
	server_settings_t< Traits > result;

	configurator( result );

	return result;
}

} /* namespace restinio */

