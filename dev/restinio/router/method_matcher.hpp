/*
 * RESTinio
 */

/**
 * @file
 * @brief Stuff related to method_matchers.
 *
 * @since v.0.6.6
 */

//FIXME: this file should be added to CMakeLists.txt

#pragma once

#include <restinio/http_headers.hpp>

#include <initializer_list>

namespace restinio
{

namespace router
{

//
// method_matcher_t
//
//FIXME: document this!
struct method_matcher_t
{
	virtual ~method_matcher_t() = default;

	RESTINIO_NODISCARD
	virtual bool
	match( const http_method_id_t & method ) const noexcept = 0;
};

namespace impl
{

//FIXME: is this code really needed?
#if 0
//
// nonallocated_matcher_proxy_t
//
//FIXME: document this!
template< typename Matcher >
class nonallocated_matcher_proxy_t : public method_matcher_t
{
	Matcher m_matcher;

public :
	template< typename... Args >
	nonallocated_matcher_proxy_t( Args && ...args )
		:	m_matcher{ std::forward<Args>(args)... }
	{}

	RESTINIO_NODISCARD
	bool
	match( const http_method_id_t & method ) const noexcept
	{
		return m_matcher.match( method );
	}
};
#endif

//
// allocated_matcher_proxy_t
//
//FIXME: document this!
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
//FIXME: document this!
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
//FIXME: document this!
template< std::size_t Size >
class fixed_size_any_of_matcher_t : public method_matcher_t
{
	std::array< http_method_id_t, Size > m_methods;

public :
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
// fixed_size_no_one_of_matcher_t
//
//FIXME: document this!
template< std::size_t Size >
class fixed_size_no_one_of_matcher_t
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
//FIXME: document this!
class buffered_matcher_holder_t
{
	static constexpr std::size_t buffer_size =
			sizeof(fixed_size_any_of_matcher_t<4>);

	static constexpr std::size_t alignment =
			std::max( {
					alignof(simple_matcher_t),
					alignof(fixed_size_any_of_matcher_t<4>),
					alignof(allocated_matcher_proxy_t<
							fixed_size_any_of_matcher_t<20>>) } );

	using pfn_move_t = method_matcher_t* (*)(void*, void*);

	method_matcher_t * m_matcher{ nullptr };
	alignas(alignment) std::array<char, buffer_size> m_buffer;
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

	RESTINIO_NODISCARD
	method_matcher_t *
	get() const noexcept { return m_matcher; }

	RESTINIO_NODISCARD
	method_matcher_t *
	operator->() const noexcept { return m_matcher; }

	RESTINIO_NODISCARD
	method_matcher_t &
	operator*() const noexcept { return *m_matcher; }
};

} /* namespace impl */

//
// any_of_methods
//
//FIXME: document this!
template< typename... Args >
RESTINIO_NODISCARD
impl::fixed_size_any_of_matcher_t< sizeof...(Args) >
any_of_methods( Args && ...args )
{
	return { std::initializer_list<http_method_id_t>{ std::forward<Args>(args)... } };
}

//
// no_one_of_methods
//
//FIXME: document this!
template< typename... Args >
RESTINIO_NODISCARD
impl::fixed_size_no_one_of_matcher_t< sizeof...(Args) >
no_one_of_methods( Args && ...args )
{
	return { std::initializer_list<http_method_id_t>{ std::forward<Args>(args)... } };
}

} /* namespace router */

} /* namespace restinio */

