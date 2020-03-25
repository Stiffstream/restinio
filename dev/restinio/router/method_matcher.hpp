/*
 * RESTinio
 */

/**
 * @file
 * @brief Stuff related to method_matchers.
 *
 * @since v.0.6.6
 */

#pragma once

#include <restinio/http_headers.hpp>

#include <initializer_list>
#include <vector>

namespace restinio
{

namespace router
{

//
// method_matcher_t
//
/*!
 * @brief An interface of method_matcher.
 *
 * Method_matchers are used by routers to detect an applicability
 * of an incoming request to a route. If method_matcher_t::match()
 * returns `true` then HTTP method from an incoming request is
 * applicable to a route.
 *
 * @since v.0.6.6
 */
struct method_matcher_t
{
	method_matcher_t( const method_matcher_t & ) = default;
	method_matcher_t &
	operator=( const method_matcher_t & ) = default;

	method_matcher_t( method_matcher_t && ) = default;
	method_matcher_t &
	operator=( method_matcher_t && ) = default;

	method_matcher_t() = default;
	virtual ~method_matcher_t() = default;

	//! Is the specified method can be applied to a route?
	/*!
	 * \retval true if @a method can be applied to a route.
	 * \retval false if @a method can't be applied to a route.
	 */
	RESTINIO_NODISCARD
	virtual bool
	match( const http_method_id_t & method ) const noexcept = 0;
};

namespace impl
{

//
// allocated_matcher_proxy_t
//
/*!
 * @brief A proxy for actual method_matcher that will be allocated
 * in dynamic memory.
 *
 * An instance of Matcher class will be allocated automatically in
 * the constructor of allocated_matcher_proxy_t.
 *
 * @note
 * This is a moveable class.
 *
 * @since v.0.6.6
 */
template< typename Matcher >
class allocated_matcher_proxy_t : public method_matcher_t
{
	std::unique_ptr< Matcher > m_matcher;

public :
	template< typename... Args >
	allocated_matcher_proxy_t( Args && ...args )
		:	m_matcher{ std::make_unique<Matcher>( std::forward<Args>(args)... ) }
	{}

	RESTINIO_NODISCARD
	bool
	match( const http_method_id_t & method ) const noexcept override
	{
		return m_matcher->match( method );
	}
};

//
// simple_matcher_t
//
/*!
 * @brief A simple method_matcher that compares just one user-specified
 * value.
 *
 * The allowed value is specified in the constructor and can't be changed
 * after that.
 *
 * @since v.0.6.6
 */
class simple_matcher_t : public method_matcher_t
{
	http_method_id_t m_method;

public :
	simple_matcher_t( http_method_id_t method )
		:	m_method{ std::move(method) }
	{}

	RESTINIO_NODISCARD
	bool
	match( const http_method_id_t & method ) const noexcept override
	{
		return m_method == method;
	}
};

//
// fixed_size_any_of_matcher_t
//
/*!
 * @brief A matcher that finds a value in the vector of allowed values
 * of fixed size.
 *
 * A method is allowed if it's found in the vector of allowed values.
 *
 * @since v.0.6.6
 */
template< std::size_t Size >
class fixed_size_any_of_matcher_t : public method_matcher_t
{
	std::array< http_method_id_t, Size > m_methods;

public :
	/*!
	 * @brief Initializing constructor.
	 *
	 * @attention
	 * The values.size() is expected to be equal to Size. The behavior is
	 * undefined otherwise.
	 */
	fixed_size_any_of_matcher_t(
		std::initializer_list< http_method_id_t > values )
	{
		assert( Size == values.size() );

		std::copy( values.begin(), values.end(), m_methods.begin() );
	}

	RESTINIO_NODISCARD
	bool
	match( const http_method_id_t & method ) const noexcept override
	{
		for( const auto & m : m_methods )
			if( m == method )
				return true;

		return false;
	}
};

//
// fixed_size_none_of_matcher_t
//
/*!
 * @brief A matcher that finds a value in the vector of disabled values
 * of fixed size.
 *
 * A method is allowed if it isn't found in the vector of disabled values.
 *
 * @since v.0.6.6
 */
template< std::size_t Size >
class fixed_size_none_of_matcher_t
	: public fixed_size_any_of_matcher_t<Size>
{
	using base_type_t = fixed_size_any_of_matcher_t<Size>;

public :
	using base_type_t::base_type_t;

	RESTINIO_NODISCARD
	bool
	match( const http_method_id_t & method ) const noexcept override
	{
		return !base_type_t::match( method );
	}
};

//
// buffered_matcher_holder_t
//
/*!
 * @brief A special class that allows to hold a copy of small-size
 * method_matchers or a pointer to dynamically allocated large-size
 * method_matchers.
 *
 * An instance of this class looks like a smart pointer to
 * method_matcher_t. This smart pointer is moveable, but not
 * copyable (it's like unique_ptr).
 *
 * A value is set by assign() method:
 * @code
 * buffered_matcher_holder_t matcher;
 * matcher.assign<fixed_size_any_of_matcher_t<10>>(
 * 	std::initializer_list<http_method_id_t>{
 * 		http_method_get(),
 * 		http_method_head(),
 * 		...
 * 	});
 * @endcode
 *
 * @since v.0.6.6
 */
class buffered_matcher_holder_t
{
	//! The size of the internal buffer.
	static constexpr std::size_t buffer_size =
			sizeof(fixed_size_any_of_matcher_t<4>);

	//! Alignment to be used by the internal buffer.
	static constexpr std::size_t alignment =
			std::max( {
					alignof(simple_matcher_t),
					alignof(fixed_size_any_of_matcher_t<4>),
					alignof(allocated_matcher_proxy_t<
							fixed_size_any_of_matcher_t<20>>) } );

	//! A type of free function to be used to move a value of
	//! an object to the specified buffer.
	/*!
	 * This function should allocate a new instance in @a buffer and
	 * move the content of @a object into it. The pointer to the allocated
	 * instance should be returned.
	 */
	using pfn_move_t = method_matcher_t* (*)(void* object, void* buffer);

	//! A pointer to actual matcher allocated inside the internall buffer.
	/*!
	 * Can be nullptr. For example: just after the creation and before
	 * the call to assign(). Or after a move-constructor or move-operator.
	 */
	method_matcher_t * m_matcher{ nullptr };

	//! The internal buffer.
	alignas(alignment) std::array<char, buffer_size> m_buffer;

	//! An actual move-function.
	/*!
	 * Can be nullptr if assign() is not called yet.
	 */
	pfn_move_t m_mover{ nullptr };

	void
	cleanup()
	{
		if( m_matcher ) m_matcher->~method_matcher_t();
	}

	void
	move_from( buffered_matcher_holder_t & other )
	{
		if( other.m_matcher )
		{
			m_matcher = other.m_mover( other.m_matcher, m_buffer.data() );
			m_mover = other.m_mover;
			
			other.m_matcher = nullptr;
			other.m_mover = nullptr;
		}
	}

public :
	buffered_matcher_holder_t() = default;

	~buffered_matcher_holder_t() noexcept
	{
		cleanup();
	}

	buffered_matcher_holder_t(
		const buffered_matcher_holder_t & ) = delete;

	buffered_matcher_holder_t &
	operator=(
		const buffered_matcher_holder_t & ) = delete;

	buffered_matcher_holder_t(
		buffered_matcher_holder_t && other ) noexcept
	{
		move_from( other );
	}

	buffered_matcher_holder_t &
	operator=( buffered_matcher_holder_t && other ) noexcept
	{
		if( this != &other )
		{
			cleanup();
			move_from( other );
		}

		return *this;
	}

	/*!
	 * @brief Creates an instance of Target_Type and initializes it
	 * with arguments Args.
	 *
	 * Previous value of buffered_matcher_holder_t will be destroyed.
	 *
	 * A new object is created in the internal buffer if its size is
	 * not greater than buffer_size. Otherwise a new object is created
	 * in dynamic memory and allocated_matcher_proxy_t for it
	 * is placed into the internal buffer.
	 *
	 */
	template< typename Target_Type, typename... Args >
	void
	assign( Args &&... args )
	{
		static_assert( alignof(Target_Type) <= alignment,
				"Target_Type should have appropriate alignment" );

		cleanup();

		if( sizeof(Target_Type) <= buffer_size )
		{
			m_matcher = new(m_buffer.data()) Target_Type{ std::forward<Args>(args)... };
			m_mover = [](void * raw_what, void * dest_storage) -> method_matcher_t * {
				auto * what = reinterpret_cast<Target_Type *>(raw_what);
				return new(dest_storage) Target_Type{ std::move(*what) };
			};
		}
		else
		{
			using actual_type = allocated_matcher_proxy_t<Target_Type>;
			m_matcher = new(m_buffer.data()) actual_type{ std::forward<Args>(args)... };
			m_mover = [](void * raw_what, void * dest_storage) -> method_matcher_t * {
				auto * what = reinterpret_cast<actual_type *>(raw_what);
				return new(dest_storage) actual_type{ std::move(*what) };
			};
		}
	}

	//! Get the pointer to actual matcher inside the holder.
	RESTINIO_NODISCARD
	method_matcher_t *
	get() const noexcept { return m_matcher; }

	//! Get the pointer to actual matcher inside the holder.
	RESTINIO_NODISCARD
	method_matcher_t *
	operator->() const noexcept { return m_matcher; }

	//! Get a reference to actual matcher inside the holder.
	RESTINIO_NODISCARD
	method_matcher_t &
	operator*() const noexcept { return *m_matcher; }

	friend void
	assign( buffered_matcher_holder_t & holder, http_method_id_t method )
	{
		holder.assign< simple_matcher_t >( std::move(method) );
	}

	template< typename Arg >
	friend void
	assign( buffered_matcher_holder_t & holder, Arg && method_matcher )
	{
		using pure_method_matcher_type = std::decay_t<Arg>;

		static_assert( std::is_base_of<
				method_matcher_t, pure_method_matcher_type >::value,
				"Arg should be derived from method_matcher_t" );

		holder.assign< pure_method_matcher_type >(
				std::forward<Arg>(method_matcher) );
	}
};

} /* namespace impl */

//
// any_of_methods
//
/*!
 * @brief A factory function that creates a method_matcher that allows
 * a method if it's found in the list of allowed methods.
 *
 * Usage example:
 * @code
 * router->add_handler(
 * 	restinio::router::any_of_methods(
 * 		restinio::http_method_get(), restinio::http_method_head()),
 * 	"/users/:id",
 * 	[](const auto & req, auto & params) {...});
 * @endcode
 *
 * @note
 * Returns the created object by value without any allocations.
 *
 * @since v.0.6.6
 */
template< typename... Args >
RESTINIO_NODISCARD
impl::fixed_size_any_of_matcher_t< sizeof...(Args) >
any_of_methods( Args && ...args )
{
	return { std::initializer_list<http_method_id_t>{ std::forward<Args>(args)... } };
}

//
// none_of_methods
//
/*!
 * @brief A factory function that creates a method_matcher that allows
 * a method if it isn't found in the list of disabled methods.
 *
 * Usage example:
 * @code
 * router->add_handler(
 * 	restinio::router::none_of_methods(
 * 		restinio::http_method_get(), restinio::http_method_head()),
 * 	"/users/:id",
 * 	[](const auto & req, auto &) {
 * 		return req->create_response(status_method_not_allowed())
 * 			.connection_close().done();
 * 	});
 * @endcode
 *
 * @note
 * Returns the created object by value without any allocations.
 *
 * @since v.0.6.6
 */
template< typename... Args >
RESTINIO_NODISCARD
impl::fixed_size_none_of_matcher_t< sizeof...(Args) >
none_of_methods( Args && ...args )
{
	return { std::initializer_list<http_method_id_t>{ std::forward<Args>(args)... } };
}

//
// dynamic_any_of_methods_matcher_t
//
/*!
 * @brief An implementation of method_matcher that allows a method
 * if it's found in a dynamic list of allowed methods.
 *
 * Usage example:
 * @code
 * restinio::router::dynamic_any_of_methods_matcher_t matcher;
 * if(config.handle_get_method())
 * 	matcher.add(restinio::http_method_get());
 * if(config.handle_head_method())
 * 	matcher.add(restinio::http_method_head());
 * router->add_handler(matcher, // Or std::move(matcher) if matcher is no more needed.
 * 	"/users/:id",
 * 	[](const auto & req, auto & params) {...});
 * @endcode
 *
 * @since v.0.6.6
 */
class dynamic_any_of_methods_matcher_t : public method_matcher_t
{
	std::vector< http_method_id_t > m_methods;

public:
	dynamic_any_of_methods_matcher_t() = default;

	RESTINIO_NODISCARD
	bool
	match( const http_method_id_t & method ) const noexcept override
	{
		for( const auto & m : m_methods )
			if( m == method )
				return true;

		return false;
	}

	dynamic_any_of_methods_matcher_t &
	add( http_method_id_t method )
	{
		m_methods.emplace_back( std::move(method) );
		return *this;
	}

	RESTINIO_NODISCARD
	std::size_t
	size() const noexcept
	{
		return m_methods.size();
	}

	RESTINIO_NODISCARD
	bool
	empty() const noexcept
	{
		return m_methods.empty();
	}
};

//
// dynamic_none_of_methods_matcher_t
//
/*!
 * @brief An implementation of method_matcher that allows a method
 * if it isn't found in a dynamic list of disabled methods.
 *
 * Usage example:
 * @code
 * restinio::router::dynamic_none_of_methods_matcher_t matcher;
 * if(config.handle_get_method())
 * 	matcher.add(restinio::http_method_get());
 * if(config.handle_head_method())
 * 	matcher.add(restinio::http_method_head());
 * router->add_handler(matcher, // Or std::move(matcher) if matcher is no more needed.
 * 	"/users/:id",
 * 	[](const auto & req, auto & params) {...});
 * @endcode
 *
 * @since v.0.6.6
 */
class dynamic_none_of_methods_matcher_t : public method_matcher_t
{
	std::vector< http_method_id_t > m_methods;

public:
	dynamic_none_of_methods_matcher_t() = default;

	RESTINIO_NODISCARD
	bool
	match( const http_method_id_t & method ) const noexcept override
	{
		for( const auto & m : m_methods )
			if( m == method )
				return false;

		return true;
	}

	dynamic_none_of_methods_matcher_t &
	add( http_method_id_t method )
	{
		m_methods.emplace_back( std::move(method) );
		return *this;
	}

	RESTINIO_NODISCARD
	std::size_t
	size() const noexcept
	{
		return m_methods.size();
	}

	RESTINIO_NODISCARD
	bool
	empty() const noexcept
	{
		return m_methods.empty();
	}
};

} /* namespace router */

} /* namespace restinio */

