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
#include <type_traits>

#include <restinio/asio_include.hpp>

#include <restinio/exception.hpp>

namespace restinio
{

namespace impl
{

//
// writable_base_t
//

//! A base class fora writable items.
class writable_base_t
{
	public:
		writable_base_t() = default;
		writable_base_t( const writable_base_t & ) = default;
		writable_base_t( writable_base_t && ) = default;
		const writable_base_t & operator = ( const writable_base_t & ) = delete;
		writable_base_t & operator = ( writable_base_t && ) = delete;

		virtual ~writable_base_t()
		{}

		//! Move this buffer enitity to a given location.
		//! \note storage must have a sufficient space and proper alignment.
		virtual void relocate_to( void * storage ) = 0;
};

//! Inernaml interface for a buffer entity.
/*!
	Having a condition to put heterogeneous buffer sequences in vector
	to transfer them from builders to connection context,
	Internal buffers are the pieces incapsulating various buffers
	implementation that fit into a fixed memory space.
	That's makes it possible to fit any of them in a binary
	buffer that resides in writable_item_t.
	While different descendant might vary in size
	size of writable_item_t remains the same, so it can be used in a vector.
*/
class buf_iface_t : public writable_base_t
{
	public:
		//! Get asio buf entity.
		virtual asio_ns::const_buffer buffer() const = 0;
<<<<<<< local

		//! Start running custo write operation.
		virtual void
		run_custom_write_operation(
			//! An executor to wrap all calls happening on asio loop.
			asio_ns::executor executor,
			 ) = 0;

		//! Move this buffer enitity to a given location.
		//! \note storage must have a sufficient space and proper alignment.
		virtual void relocate_to( void * storage ) = 0;

		buf_iface_t() = default;
		buf_iface_t( const buf_iface_t & ) = default;
		buf_iface_t( buf_iface_t && ) = default;
		const buf_iface_t & operator = ( const buf_iface_t & ) = delete;
		buf_iface_t & operator = ( buf_iface_t && ) = delete;

		virtual ~buf_iface_t() = default;
=======
>>>>>>> other
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

		virtual void relocate_to( void * storage ) override
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

		virtual void relocate_to( void * storage ) override
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

		virtual void relocate_to( void * storage ) override
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

		virtual void relocate_to( void * storage ) override
		{
			new( storage ) shared_datasizeable_buf_t{ std::move( *this ) };
		}
		//! \}

	private:
		shared_ptr_t m_buf_ptr;
};


constexpr std::size_t buffer_storage_align =
	std::max< std::size_t >( {
		alignof( empty_buf_t ),
		alignof( const_buf_t ),
		alignof( string_buf_t ),
		alignof( shared_datasizeable_buf_t< std::string > ) } );

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
// writable_item_type_t
//

//! Popped buffers write operation type.
enum class writable_item_type_t
{
	//! Item is a buffer and must be written trivially
	trivial_write_operation,

	//! Item is a sendfile operation and implicates file write operation.
	file_write_operation,

	//! Nothing to write.
	none
};

//
// writable_item_t
//

//! Class for storing the buffers used for streaming body (request/response).
class writable_item_t
{
		//! Get size of storage.
	public:
		writable_item_t( const writable_item_t & ) = delete;
		writable_item_t & operator = ( const writable_item_t & ) = delete;

		writable_item_t()
			:	m_write_type{ writable_item_type_t::trivial_write_operation }
		{
			new( &m_storage ) impl::empty_buf_t{};
		}

		writable_item_t( const_buffer_t const_buf )
			:	m_write_type{ writable_item_type_t::trivial_write_operation }
		{
			new( &m_storage ) impl::const_buf_t{ const_buf.m_str, const_buf.m_size };
		}

		writable_item_t( std::string str )
			:	m_write_type{ writable_item_type_t::trivial_write_operation }
		{
			new( &m_storage ) impl::string_buf_t{ std::move( str ) };
		}

		writable_item_t( const char * str )
			// We can't be sure whether it is valid to consider
			// data pointed by str a const buffer, so we make a string copy here.
			:	writable_item_t{ std::string{ str } }
		{}

		template < typename T >
		writable_item_t( std::shared_ptr< T > sp )
			:	m_write_type{ writable_item_type_t::trivial_write_operation }
		{
			static_assert(
				sizeof( std::shared_ptr< T > ) <= impl::needed_storage_max_size,
				"size of shared_ptr on a type is too big" );

			if( !sp )
				throw exception_t{ "empty shared_ptr cannot be used as buffer" };

			new( &m_storage ) impl::shared_datasizeable_buf_t< T >{ std::move( sp ) };
		}

		writable_item_t( writable_item_t && b )
			:	m_write_type{ b.m_write_type }
		{
			b.get_buf()->relocate_to( &m_storage );
		}

		void
		operator = ( writable_item_t && b )
		{
			if( this != &b )
			{
				destroy_stored_buffer();
				m_write_type = b.m_write_type;
				b.get_buf()->relocate_to( &m_storage );
			}
		}

		~writable_item_t()
		{
			destroy_stored_buffer();
		}

		writable_item_type_t
		write_type() const
		{
			return m_write_type;
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
			using dtor_writable_base_t = impl::writable_base_t;
			get_buf()->~dtor_writable_base_t();
		}

		writable_item_type_t m_write_type;

		//! Access buf item.
		//! \{
		const impl::buf_iface_t * get_buf() const
		{
			return reinterpret_cast< const impl::buf_iface_t * >( &m_storage );
		}

		impl::buf_iface_t * get_buf()
		{
			return reinterpret_cast< impl::buf_iface_t * >( &m_storage );
		}

		using storage_t =
			std::aligned_storage_t<
				impl::needed_storage_max_size,
				impl::buffer_storage_align >;

		storage_t m_storage;
		//! \}
};

//! Back compatibility.
//! \deprecated Obsolete in v.4.2.0. Use writable_item_t instead.
// using buffer_storage_t = writable_item_t;

//
// writable_items_container_t
//

using writable_items_container_t = std::vector< writable_item_t >;

//! Back compatibility.
//! \deprecated Obsolete in v.4.2.0. Use writable_item_t instead.
// using buffers_container_t = writable_items_container_t;

} /* namespace restinio */
