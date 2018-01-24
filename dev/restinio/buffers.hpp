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

#include <restinio/asio_include.hpp>

#include <restinio/exception.hpp>

namespace restinio
{

namespace impl
{

//! Inernaml interface for a buffer entity.
/*!
	Having a condition to put heterogeneous buffer sequences in vector
	to transfer them from builders to connection context,
	Internal buffers are the pieces incapsulating various buffers
	implementation that fit into a fixed memory space.
	That's makes it possible to fit any of them in a binary
	buffer that resides in buffer_storage_t.
	While different descendant might vary in size
	size of buffer_storage_t remains the same, so it can be used in a vector.
*/
class buf_iface_t
{
	public:
		//! Get asio buf entity.
		virtual asio_ns::const_buffer buffer() const = 0;

		//! Move this buffer enitity to a given location.
		//! \note storage must have a sufficient space and proper alignment.
		virtual void move_to( void * storage ) = 0;

		buf_iface_t() = default;
		buf_iface_t( const buf_iface_t & ) = default;
		buf_iface_t( buf_iface_t && ) = default;
		const buf_iface_t & operator = ( const buf_iface_t & ) = delete;
		buf_iface_t & operator = ( buf_iface_t && ) = delete;

		virtual ~buf_iface_t() = default;
};

//! Empty buffer entity.
class empty_buf_t final : public buf_iface_t
{
	public:
		empty_buf_t() {}

		empty_buf_t( const empty_buf_t & ) = delete;
		const empty_buf_t & operator = ( const empty_buf_t & ) = delete;

		empty_buf_t( empty_buf_t && ) = default; // allow only explicit move.
		empty_buf_t & operator = ( empty_buf_t && ) = delete;

		//! Implement buf_iface_t.
		//! \{
		virtual asio_ns::const_buffer buffer() const override
		{
			return asio_ns::const_buffer{ nullptr, 0 };
		}

		virtual void move_to( void * storage ) override
		{
			new( storage ) empty_buf_t{};
		}
		//! \}
};

//! Buffer entity for const buffer.
class const_buf_t final : public buf_iface_t
{
	public:
		const_buf_t() = delete;

		const_buf_t( const void * data, std::size_t size )
			:	m_data{ data }
			,	m_size{ size }
		{}

		const_buf_t( const const_buf_t & ) = delete;
		const const_buf_t & operator = ( const const_buf_t & ) = delete;

		const_buf_t( const_buf_t && ) = default; // allow only explicit move.
		const_buf_t & operator = ( const_buf_t && ) = delete;

		//! Implement buf_iface_t.
		//! \{
		virtual asio_ns::const_buffer buffer() const override
		{
			return asio_ns::const_buffer{ m_data, m_size };
		}

		virtual void move_to( void * storage ) override
		{
			new( storage ) const_buf_t{ std::move( *this ) };
		}
		//! \}

	private:
		const void * m_data;
		std::size_t m_size;
};

//! Buffer entity based on std::string.
class string_buf_t final : public buf_iface_t
{
	public:
		string_buf_t() = delete;
		string_buf_t( std::string buf )
			:	m_buf{ std::move( buf ) }
		{}

		string_buf_t( const string_buf_t & ) = delete;
		const string_buf_t & operator = ( const string_buf_t & ) = delete;

		string_buf_t( string_buf_t && ) = default; // allow only explicit move.
		string_buf_t & operator = ( string_buf_t && ) = delete;

		//! Implement buf_iface_t.
		//! \{
		virtual asio_ns::const_buffer buffer() const override
		{
			return asio_ns::const_buffer{ m_buf.data(), m_buf.size() };
		}

		virtual void move_to( void * storage ) override
		{
			new( storage ) string_buf_t{ std::move( *this ) };
		}
		//! \}

	private:
		std::string m_buf;
};

//! Buffer entity based on shared_ptr of data-sizeable entity.
template < typename T >
class shared_datasizeable_buf_t final : public buf_iface_t
{
	public:
		using shared_ptr_t = std::shared_ptr< T >;

		shared_datasizeable_buf_t() = delete;

		shared_datasizeable_buf_t( shared_ptr_t buf_ptr )
			:	m_buf_ptr{ std::move( buf_ptr ) }
		{}

		shared_datasizeable_buf_t( const shared_datasizeable_buf_t & ) = delete;
		const shared_datasizeable_buf_t & operator = ( const shared_datasizeable_buf_t & ) = delete;

		shared_datasizeable_buf_t( shared_datasizeable_buf_t && ) = default; // allow only explicit move.
		shared_datasizeable_buf_t & operator = ( shared_datasizeable_buf_t && ) = delete;

		//! Implement buf_iface_t.
		//! \{
		virtual asio_ns::const_buffer buffer() const override
		{
			return asio_ns::const_buffer{ m_buf_ptr->data(), m_buf_ptr->size() };
		}

		virtual void move_to( void * storage ) override
		{
			new( storage ) shared_datasizeable_buf_t{ std::move( *this ) };
		}
		//! \}

	private:
		shared_ptr_t m_buf_ptr;
};


constexpr std::size_t buffer_storage_align = alignof( buf_iface_t );

//! An amount of memory that is to be enough to hold any possible buffer entity.
constexpr std::size_t needed_storage_max_size =
	std::max< std::size_t >( {
		sizeof( empty_buf_t ),
		sizeof( const_buf_t ),
		sizeof( string_buf_t ),
		sizeof( shared_datasizeable_buf_t< std::string > ) } );

} /* namespace impl */

//
// const_buffer_t
//

//! Helper class for setting a constant buffer storage explicitly.
struct const_buffer_t
{
	const_buffer_t(
		const void * str,
		std::size_t size )
		:	m_str{ str }
		,	m_size{ size }
	{}

	const void * const m_str;
	const std::size_t m_size;
};

//! Create const buffers
//! \{
inline const_buffer_t
const_buffer( const void * str, std::size_t size )
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
class alignas( impl::buffer_storage_align ) buffer_storage_t
{
		//! Get size of storage.
	public:
		buffer_storage_t( const buffer_storage_t & ) = delete;
		buffer_storage_t & operator = ( const buffer_storage_t & ) = delete;

		buffer_storage_t()
		{
			new( m_storage.data() ) impl::empty_buf_t{};
		}

		buffer_storage_t( const_buffer_t const_buf )
		{
			new( m_storage.data() ) impl::const_buf_t{ const_buf.m_str, const_buf.m_size };
		}

		buffer_storage_t( std::string str )
		{
			new( m_storage.data() ) impl::string_buf_t{ std::move( str ) };
		}

		buffer_storage_t( const char * str )
		{
			// We can't be sure whether it is valid to consider
			// data pointed by str a const buffer, so we make a strin copy here.
			new( m_storage.data() ) impl::string_buf_t{ std::string{ str } };
		}

		template < typename T >
		buffer_storage_t( std::shared_ptr< T > sp )
		{
			static_assert(
				sizeof( std::shared_ptr< T > ) <= impl::needed_storage_max_size,
				"size of shared_ptr on a type is too big" );

			if( !sp )
				throw exception_t{ "empty shared_ptr cannot be used as buffer" };

			new( m_storage.data() ) impl::shared_datasizeable_buf_t< T >{ std::move( sp ) };
		}

		buffer_storage_t( buffer_storage_t && b )
		{
			b.get_buf()->move_to( m_storage.data() );
		}

		void
		operator = ( buffer_storage_t && b )
		{
			if( this != &b )
			{
				destroy_stored_buffer();
				b.get_buf()->move_to( m_storage.data() );
			}
		}

		~buffer_storage_t()
		{
			destroy_stored_buffer();
		}

		asio_ns::const_buffer
		buf() const
		{
			return get_buf()->buffer();
		}

	private:
		void
		destroy_stored_buffer()
		{
			using dtor_buf_iface_t = impl::buf_iface_t;
			get_buf()->~dtor_buf_iface_t();
		}

		//! Access buf item.
		//! \{
		const impl::buf_iface_t * get_buf() const
		{
			return reinterpret_cast< const impl::buf_iface_t * >( m_storage.data() );
		}

		impl::buf_iface_t * get_buf()
		{
			return reinterpret_cast< impl::buf_iface_t * >( m_storage.data() );
		}

		alignas( impl::buffer_storage_align )
		std::array< char, impl::needed_storage_max_size > m_storage;
		//! \}
};

//
// buffers_container_t
//

using buffers_container_t = std::vector< buffer_storage_t >;

} /* namespace restinio */
