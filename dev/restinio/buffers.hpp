/*
	restinio
*/

/*!
	Restinio buffers.
*/

#pragma once

#include <memory>
#include <array>
#include <string>
#include <cstring>

#include <asio/buffer.hpp>

#include <restinio/exception.hpp>

namespace restinio
{

namespace impl
{

//! Helper function to get the reference on expected type out of `void*`.
template < typename T >
T &
buf_access( void * p )
{
	T * sp = reinterpret_cast< T * >( p );
	return *sp;
}

template < typename T >
const T &
buf_access( const void * p )
{
	const T * sp = reinterpret_cast< const T * >( p );
	return *sp;
}

constexpr std::size_t needed_storage_max_size =
	sizeof( std::string ) > sizeof( std::shared_ptr< std::string > ) ?
		sizeof( std::string ) : sizeof( std::shared_ptr< std::string > );
} /* namespace impl */

//
// const_buffer_t
//

//! Helper class for setting a constant buffer storage explicitly.
struct const_buffer_t
{
	const_buffer_t(
		const char * str,
		std::size_t size )
		:	m_str{ str }
		,	m_size{ size }
	{}

	const char * const m_str;
	const std::size_t m_size;
};

//! Create const buffers
//! \{
inline const_buffer_t
const_buffer( const char * str, std::size_t size )
{
	return const_buffer_t{ str, size };
}

inline const_buffer_t
const_buffer( const char * str )
{
	return const_buffer( str, std::strlen( str ) );
}
//! \}

//
// buffer_storage_t
//

//! Class for storing the buffers used for streaming body (request/response).
class alignas( std::max_align_t ) buffer_storage_t
{
		//! Get size of storage.
	public:
		buffer_storage_t( const buffer_storage_t & ) = delete;
		void operator = ( const buffer_storage_t & ) = delete;

		buffer_storage_t()
			:	m_accessor{
					[]( const void * ){ return asio::const_buffer{ nullptr, 0 }; } }
			,	m_move{ []( const void * , void * ){} }
		{}

		buffer_storage_t( const_buffer_t const_buf )
			:	m_accessor{
					[]( const void * p ){
						const auto s = impl::buf_access< const char * >( p );
						const auto n = impl::buf_access< std::size_t >(
								(const char *)p + sizeof( const char *) );

						return asio::const_buffer{ s, n };
					} }
			,	m_move{
					[]( const void * src, void * dest ){
						const char * s = impl::buf_access< const char * >( src );
						const std::size_t n = impl::buf_access< std::size_t >(
								(const char *)src + sizeof( const char *) );

						impl::buf_access< const char * >( dest ) = s;
						impl::buf_access< std::size_t >( (char *)dest + sizeof( const char *) ) = n;
					} }
		{
			auto * p = m_storage.data();
			impl::buf_access< const char * >( p ) = const_buf.m_str;
			impl::buf_access< std::size_t >( (char *)p + sizeof( const char *) ) = const_buf.m_size;
		}

		buffer_storage_t( std::string str )
			:	m_accessor{
					[]( const void * p ){
						const auto & s = impl::buf_access< std::string >( p );
						return asio::const_buffer{ s.data(), s.size() };
					} }
			,	m_move{
					[]( const void * src, void * dest ){
						auto & s = impl::buf_access< std::string >( src );

						new( dest ) std::string{ std::move( s ) };
					} }
			,	m_destructor{
					[]( void * p ){
						auto & s = impl::buf_access< std::string >( p );

						using namespace std;
						s.string::~string();
					} }
		{
			new( m_storage.data() ) std::string{ std::move( str ) };
		}

		buffer_storage_t( const char * str )
			:	buffer_storage_t{ std::string{ str } }
		{}

		template < typename T >
		buffer_storage_t( std::shared_ptr< T > sp )
			:	m_accessor{
					[]( const void * p ){
						const auto & sp = impl::buf_access< std::shared_ptr< T > >( p );
						return asio::const_buffer{ sp->data(), sp->size() };
					} }
			,	m_move{
					[]( const void * src, void * dest ){
						auto & sp = impl::buf_access< std::shared_ptr< T > >( src );

						new( dest ) std::shared_ptr< T >{ std::move( sp ) };
					} }
			,	m_destructor{
					[]( void * p ){
						auto & sp = impl::buf_access< std::shared_ptr< T > >( p );
						sp.~shared_ptr();
					} }
		{
			static_assert(
				sizeof( std::shared_ptr< T > ) <= impl::needed_storage_max_size,
				"size of shared_ptr on a type is too big" );

			if( !sp )
				throw exception_t{ "ampty shared_ptr cannot be used as buffer" };

			new( m_storage.data() ) std::shared_ptr< T >{ std::move( sp ) };
		}

		buffer_storage_t( buffer_storage_t && b )
			:	m_accessor{ b.m_accessor }
			,	m_move{ b.m_move }
			,	m_destructor{ b.m_destructor }
		{
			(*m_move)( b.m_storage.data(), m_storage.data() );
		}

		void
		operator = ( buffer_storage_t && b )
		{
			if( this != &b )
			{
				destroy_stored_buffer();
				m_accessor = b.m_accessor;
				m_move = b.m_move;
				m_destructor = b.m_destructor;

				(*m_move)( b.m_storage.data(), m_storage.data() );
			}
		}

		~buffer_storage_t()
		{
			destroy_stored_buffer();
		}

		asio::const_buffer
		buf() const
		{
			assert( nullptr != m_accessor );

			return (*m_accessor)( m_storage.data() );
		}

	private:
		void
		destroy_stored_buffer()
		{
			if( nullptr != m_destructor )
				(*m_destructor)( m_storage.data() );
		}

		alignas( std::max_align_t ) std::array< char, impl::needed_storage_max_size > m_storage;

		using buffer_accessor_func_t = asio::const_buffer (*)( const void * );
		using buffer_move_func_t = void (*)( const void *, void * );
		using buffer_destructor_func_t = void (*)( void * );

		buffer_accessor_func_t m_accessor{ nullptr };
		buffer_move_func_t m_move{ nullptr };
		buffer_destructor_func_t m_destructor{ nullptr };
};

//
// buffers_container_t
//

using buffers_container_t = std::vector< buffer_storage_t >;

} /* namespace restinio */
