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
#include <restinio/sendfile.hpp>

#include <restinio/compiler_features.hpp>
#include <restinio/utils/suppress_exceptions.hpp>

#include <restinio/impl/include_fmtlib.hpp>


namespace restinio
{

//
// fmt_minimal_memory_buffer_t
//
/*!
 * @brief An alias for fmt::basic_memory_buffer<char,1>.
 *
 * @since v.0.6.3
 */
using fmt_minimal_memory_buffer_t = fmt::basic_memory_buffer<char, 1u>;

namespace impl
{

//
// writable_base_t
//

//! A base class for writable items.
/*!
	Having a condition to put heterogeneous writable-items sequence in vector
	and to transfer it from builders to connection context,
	internal writable-items are the pieces encapsulating various
	implementation that fit into a fixed memory space.
	That's makes it possible to fit any of them in a binary
	buffer that resides in writable_item_t.
	While different descendants might vary in size
	size of writable_item_t remains the same, so it can be used in a vector.
*/
class writable_base_t
{
	public:
		writable_base_t() = default;
		writable_base_t( const writable_base_t & ) = default;
		writable_base_t( writable_base_t && ) = default;
		writable_base_t & operator = ( const writable_base_t & ) = delete;
		writable_base_t & operator = ( writable_base_t && ) = delete;

		virtual ~writable_base_t()
		{}

		//! Move this buffer enitity to a given location.
		//! \note storage must have a sufficient space and proper alignment.
		virtual void relocate_to( void * storage ) = 0;

		//! Get the size of a writable piece of data.
		virtual std::size_t size() const = 0;
};

//! Internal interface for a trivial buffer-like entity.
class buf_iface_t : public writable_base_t
{
	public:
		//! Get asio buf entity.
		/*!
			Prepares an item for being used with ASIO API.
		*/
		virtual asio_ns::const_buffer buffer() const = 0;
};

//! Empty buffer entity.
class empty_buf_t final : public buf_iface_t
{
	public:
		empty_buf_t() noexcept {}

		empty_buf_t( const empty_buf_t & ) = delete;
		empty_buf_t & operator = ( const empty_buf_t & ) = delete;

		empty_buf_t( empty_buf_t && ) = default; // allow only explicit move.
		empty_buf_t & operator = ( empty_buf_t && ) = delete;

		/*!
			@name An implementation of writable_base_t interface.

			\see writable_base_t
		*/
		///@{
		virtual asio_ns::const_buffer buffer() const override
		{
			return asio_ns::const_buffer{ nullptr, 0 };
		}

		virtual void relocate_to( void * storage ) override
		{
			new( storage ) empty_buf_t{};
		}
		///@}

		/*!
			@name An implementation of buf_iface_t interface.

			\see buf_iface_t
		*/
		///@{
		virtual std::size_t size() const override { return  0; }
		///@}
};

//! Buffer entity for const buffer.
class const_buf_t final : public buf_iface_t
{
	public:
		const_buf_t() = delete;

		constexpr const_buf_t( const void * data, std::size_t size ) noexcept
			:	m_data{ data }
			,	m_size{ size }
		{}

		const_buf_t( const const_buf_t & ) = delete;
		const_buf_t & operator = ( const const_buf_t & ) = delete;

		const_buf_t( const_buf_t && ) = default; // allow only explicit move.
		const_buf_t & operator = ( const_buf_t && ) = delete;

		/*!
			@name An implementation of writable_base_t interface.

			\see writable_base_t
		*/
		///@{
		virtual asio_ns::const_buffer buffer() const override
		{
			return asio_ns::const_buffer{ m_data, m_size };
		}

		virtual void relocate_to( void * storage ) override
		{
			new( storage ) const_buf_t{ std::move( *this ) };
		}
		///@}

		/*!
			@name An implementation of buf_iface_t interface.

			\see buf_iface_t
		*/
		///@{
		virtual std::size_t size() const override { return m_size; }
		///@}

	private:
		//! A pointer to data.
		const void * const m_data;
		//! The size of data.
		const std::size_t m_size;
};

//! User defined datasizable object.
/*!
	\note there is a limitation on how large a `Datasizeable` type can be.
	The limitation is checked with a following predicate:
	\code
		sizeof(datasizeable_buf_t<D>) <= needed_storage_max_size;
	\endcode
*/
template < typename Datasizeable >
class datasizeable_buf_t final : public buf_iface_t
{
	// Check datasizeable contract:
	static_assert(
		std::is_convertible<
				decltype( std::declval< const Datasizeable >().data() ),
				const void *
			>::value,
			"Datasizeable requires 'T* data() const' member function, "
			"where 'T*' is convertible to 'void*' " );

	static_assert(
		std::is_convertible<
				decltype( std::declval< const Datasizeable >().size() ),
				std::size_t
			>::value,
			"Datasizeable requires 'N size() const' member function, "
			"where 'N' is convertible to 'std::size_t'" );

	static_assert(
		std::is_move_constructible< Datasizeable >::value,
			"Datasizeable must be move constructible" );

	public:
		datasizeable_buf_t( Datasizeable buf )
			:	m_custom_buffer{ std::move( buf ) }
		{}

		datasizeable_buf_t( datasizeable_buf_t && ) noexcept = default; // allow only explicit move.

		/*!
			@name An implementation of writable_base_t interface.

			\see writable_base_t
		*/
		///@{
		virtual asio_ns::const_buffer buffer() const override
		{
			return asio_ns::const_buffer{
				m_custom_buffer.data(),
				m_custom_buffer.size() };
		}

		virtual void relocate_to( void * storage ) override
		{
			new( storage ) datasizeable_buf_t{ std::move( *this ) };
		}
		///@}

		/*!
			@name An implementation of buf_iface_t interface.

			\see buf_iface_t
		*/
		///@{
		virtual std::size_t size() const override { return m_custom_buffer.size(); }
		///@}

	private:
		//! A datasizeable item that represents buffer.
		Datasizeable m_custom_buffer;
};

//! An alias for a std::string instantiation of datasizeable_buf_t<D> template.
/*!
	Used to figure out buffer_storage_align and needed_storage_max_size
	constants.
*/
using string_buf_t = datasizeable_buf_t< std::string >;

//! An alias for a fmt_minimal_memory_buffer_t instantiation of datasizeable_buf_t<D> template.
/*!
	Used to figure out buffer_storage_align and needed_storage_max_size
	constants.
*/
using fmt_minimal_memory_buffer_buf_t =
		datasizeable_buf_t< fmt_minimal_memory_buffer_t >;

//
// shared_datasizeable_buf_t
//

//! Buffer based on shared_ptr of data-sizeable entity.
template < typename Datasizeable >
class shared_datasizeable_buf_t final : public buf_iface_t
{
	public:
		using shared_ptr_t = std::shared_ptr< Datasizeable >;

		shared_datasizeable_buf_t() = delete;

		shared_datasizeable_buf_t( shared_ptr_t buf_ptr ) noexcept
			:	m_buf_ptr{ std::move( buf_ptr ) }
		{}

		shared_datasizeable_buf_t( const shared_datasizeable_buf_t & ) = delete;
		shared_datasizeable_buf_t & operator = ( const shared_datasizeable_buf_t & ) = delete;

		shared_datasizeable_buf_t( shared_datasizeable_buf_t && ) noexcept = default; // allow only explicit move.
		shared_datasizeable_buf_t & operator = ( shared_datasizeable_buf_t && ) = delete;

		/*!
			@name An implementation of writable_base_t interface.

			\see writable_base_t
		*/
		///@{
		virtual asio_ns::const_buffer buffer() const override
		{
			return asio_ns::const_buffer{ m_buf_ptr->data(), m_buf_ptr->size() };
		}

		virtual void relocate_to( void * storage ) override
		{
			new( storage ) shared_datasizeable_buf_t{ std::move( *this ) };
		}
		///@}

		/*!
			@name An implementation of buf_iface_t interface.

			\see buf_iface_t
		*/
		///@{
		virtual std::size_t size() const override { return m_buf_ptr->size(); }
		///@}

	private:
		//! A shared pointer to a datasizeable entity.
		shared_ptr_t m_buf_ptr;
};

//
// sendfile_write_operation_t
//

//! Send file operation wrapper.
struct sendfile_write_operation_t : public writable_base_t
{
	public:
		sendfile_write_operation_t() = delete;

		sendfile_write_operation_t( sendfile_t && sf_opts )
			:	m_sendfile_options{ std::make_unique< sendfile_t >( std::move( sf_opts ) ) }
		{}

		sendfile_write_operation_t( const sendfile_write_operation_t & ) = delete;
		sendfile_write_operation_t & operator = ( const sendfile_write_operation_t & ) = delete;

		sendfile_write_operation_t( sendfile_write_operation_t && ) = default;
		sendfile_write_operation_t & operator = ( sendfile_write_operation_t && ) = delete;

		/*!
			@name An implementation of writable_base_t interface.

			\see writable_base_t
		*/
		///@{
		virtual void relocate_to( void * storage ) override
		{
			new( storage ) sendfile_write_operation_t{ std::move( *this ) };
		}

		virtual std::size_t size() const override
		{
			return m_sendfile_options ? m_sendfile_options->size() : 0;
		}
		///@}

		//! Get sendfile operation detaiols.
		/*!
			@note
			Since v.0.4.9 it is non-const method. It is because we have
			to work with mutable sendfile_t on some platform (like Windows).
		*/
		sendfile_t &
		sendfile_options() noexcept
		{
			return *m_sendfile_options;
		}

	private:
		//! A pointer to sendfile operation details.
		std::unique_ptr< sendfile_t > m_sendfile_options;
};

// Constant for suitable alignment of any entity in writable_base_t hierarchy.
constexpr std::size_t buffer_storage_align =
	std::max< std::size_t >( {
		alignof( empty_buf_t ),
		alignof( const_buf_t ),
		alignof( string_buf_t ),
		alignof( shared_datasizeable_buf_t< std::string > ),
		alignof( sendfile_write_operation_t ),
		alignof( fmt_minimal_memory_buffer_buf_t ) } );

//! An of memory that is to be enough to hold any possible buffer entity.
constexpr std::size_t needed_storage_max_size =
	std::max< std::size_t >( {
		sizeof( empty_buf_t ),
		sizeof( const_buf_t ),
		sizeof( string_buf_t ),
		sizeof( shared_datasizeable_buf_t< std::string > ),
		sizeof( sendfile_write_operation_t ),
		sizeof( fmt_minimal_memory_buffer_buf_t ) } );

} /* namespace impl */

//
// const_buffer_t/

//! Helper class for setting a constant buffer storage explicitly.
/*
	A proxy DTO type.
	Its instances are emitted with const_buffer functions and
	are possible to converted to writable_item_t as it has a constructor for it.
*/
struct const_buffer_t
{
	constexpr const_buffer_t(
		const void * str,
		std::size_t size ) noexcept
		:	m_str{ str }
		,	m_size{ size }
	{}

	const void * const m_str;
	const std::size_t m_size;
};

//! @name Create const buffers.
///@{
inline constexpr const_buffer_t
const_buffer( const void * str, std::size_t size ) noexcept
{
	return const_buffer_t{ str, size };
}

inline const_buffer_t
const_buffer( const char * str ) noexcept
{
	return const_buffer( str, std::strlen( str ) );
}
///@}

//
// writable_item_type_t
//

//! Buffers write operation type.
enum class writable_item_type_t
{
	//! Item is a buffer and must be written trivially
	trivial_write_operation,

	//! Item is a sendfile operation and implicates file write operation.
	file_write_operation,
};

//
// writable_item_t
//

//! Class for storing the buffers used for streaming body (request/response).
/*!
	Supporting different types of entities that eventually result in
	output data sent to peer is a bit tricky.
	In the first step RESTionio distinguish two types of output data sources:
	  - trivial buffers (those ones that can be presented as a pair
	  of a pointer to data and the size of the data).
	  - sendfile (send a piece of data from file utilizing native
	  sendfile support Linux/FreeBSD/macOS and TransmitFile on windows).

	Also trivial buffers are implemented diferently for different cases,
	includeing a template classes `impl::datasizeable_buf_t<Datasizeable>` and
	`impl::shared_datasizeable_buf_t<Datasizeable>`.

	When using RESTinio response builder, response body can be constructed
	using different types of buffers, and once the result
	is flushed to the connection it must be sent to the socket.
	A couple of issues arises here.
	And one of them is how to store all these heterogeneous buffers
	with fewer allocations and less boilerplate necessary
	to handle each corner case.
	Storing and moving a bunch of buffers as a vector would be
	nice, but vector demands a single type to be used.
	And writable_item_t represents a custom variant-like abstraction
	for storing an arbitrary buffer object (`std::variant` itself is not the option
	as it is restricted to types defined beforehand and cannot benifit
	from the knowledge that all stored items are types derrived from
	impl::writable_base_t).

	For storing the data of buffers #m_storage is used.
	It is an aligned buffer sufficient to store
	any impl::writable_base_t descendant (there is a limitation
	concerned with impl::datasizeable_buf_t).
	Also writable_item_t exposes interface to treat it
	like a trivial buffer or a sendfile operation.
	The type of buffer currently stored in writable_item_t
	instance a write_type() function used.

	Having such writable_item_t class, RESTinio  can store a sequence
	of arbitrary buffers in `std::vector`.
*/
class writable_item_t
{
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

		template <
			typename Datasizeable,
			typename S = typename
				std::enable_if_t<
					!std::is_same<
						std::vector< writable_item_t >,
						Datasizeable >::value > >
		writable_item_t( Datasizeable ds )
			:	m_write_type{ writable_item_type_t::trivial_write_operation }
		{
			static_assert(
				sizeof( impl::datasizeable_buf_t< Datasizeable > ) <= impl::needed_storage_max_size,
				"size of type is too big" );

			new( &m_storage ) impl::datasizeable_buf_t< Datasizeable >{ std::move( ds ) };
		}

		writable_item_t( const char * str )
			// We can't be sure whether it is valid to consider
			// data pointed by str a const buffer, so we make a string copy here.
			:	writable_item_t{ std::string{ str } }
		{}

		template < typename Datasizeable >
		writable_item_t( std::shared_ptr< Datasizeable > sp )
			:	m_write_type{ writable_item_type_t::trivial_write_operation }
		{
			static_assert(
				sizeof( impl::shared_datasizeable_buf_t< Datasizeable > ) <= impl::needed_storage_max_size,
				"size of shared_ptr on a type is too big" );

			if( !sp )
				throw exception_t{ "empty shared_ptr cannot be used as buffer" };

			new( &m_storage ) impl::shared_datasizeable_buf_t< Datasizeable >{ std::move( sp ) };
		}

		writable_item_t( sendfile_t sf_opts )
			:	m_write_type{ writable_item_type_t::file_write_operation }
		{
			new( &m_storage ) impl::sendfile_write_operation_t{ std::move( sf_opts ) };
		}

		writable_item_t( writable_item_t && b )
			:	m_write_type{ b.m_write_type }
		{
			b.get_writable_base()->relocate_to( &m_storage );
		}

		writable_item_t &
		operator = ( writable_item_t && b )
		{
			if( this != &b )
			{
				destroy_stored_buffer();
				m_write_type = b.m_write_type;
				b.get_writable_base()->relocate_to( &m_storage );
			}

			return *this;
		}

		~writable_item_t()
		{
			destroy_stored_buffer();
		}

		//! Get a type of a stored buffer object.
		writable_item_type_t
		write_type() const noexcept
		{
			return m_write_type;
		}

		//! Get the size of the underlying buffer object.
		std::size_t size() const { return get_writable_base()->size(); }

		//! Create a buf reference object used by ASIO.
		/*!
			\note Stored buffer must be of writable_item_type_t::trivial_write_operation.
		*/
		asio_ns::const_buffer buf() const { return get_buf()->buffer(); }

		//! Get a reference to a sendfile operation.
		/*!
			@note Stored buffer must be of writable_item_type_t::file_write_operation.
			@note
			Since v.0.4.9 it is non-const method. It is because we have
			to work with mutable sendfile_t on some platform (like Windows).
		*/
		sendfile_t &
		sendfile_operation()
		{
			return get_sfwo()->sendfile_options();
		}

	private:
		void
		destroy_stored_buffer()
		{
			using dtor_writable_base_t = impl::writable_base_t;
			get_writable_base()->~dtor_writable_base_t();
		}

		writable_item_type_t m_write_type;

		/** @name Access an item as an object of specific types.
		 * @brief Casts a stored object to one of the types.
		*/
		///@{

		//! Access as writable_base_t item.
		const impl::writable_base_t * get_writable_base() const noexcept
		{
			return reinterpret_cast< const impl::writable_base_t * >( &m_storage );
		}

		//! Access as writable_base_t item.
		impl::writable_base_t * get_writable_base() noexcept
		{
			return reinterpret_cast< impl::writable_base_t * >( &m_storage );
		}

		//! Access as trivial buf item.
		const impl::buf_iface_t * get_buf() const noexcept
		{
			return reinterpret_cast< const impl::buf_iface_t * >( &m_storage );
		}

		//! Access as trivial buf item.
		impl::buf_iface_t * get_buf() noexcept
		{
			return reinterpret_cast< impl::buf_iface_t * >( &m_storage );
		}

		//! Access as sendfile_write_operation_t item.
		impl::sendfile_write_operation_t * get_sfwo() noexcept
		{
			return reinterpret_cast< impl::sendfile_write_operation_t * >( &m_storage );
		}
		///@}

		using storage_t =
			std::aligned_storage_t<
				impl::needed_storage_max_size,
				impl::buffer_storage_align >;

		//! A storage for a buffer object of various types.
		storage_t m_storage;
};

//
// writable_items_container_t
//

using writable_items_container_t = std::vector< writable_item_t >;

//
// write_status_cb_t
//

//! An alias for a callback to be invoked after the write operation of
//! a particular group of "buffers".
/*!
	@since v.0.4.8
*/
using write_status_cb_t =
		std::function< void( const asio_ns::error_code & ec ) >;

//
// write_group_t
//

//! Group of writable items transported to the context of underlying connection
//! as one solid piece.
/*!
	@since v.0.4.8
*/
class write_group_t
{
	public:
		//! Swap two groups.
		friend void
		swap( write_group_t & left, write_group_t & right ) noexcept
		{
			using std::swap;
			swap( left.m_items, right.m_items );
			swap( left.m_status_line_size, right.m_status_line_size );
			swap( left.m_after_write_notificator, right.m_after_write_notificator );
		}

		//! Construct write group with a given bunch of writable items.
		explicit write_group_t(
			//! A buffer objects included in this group.
			writable_items_container_t items ) noexcept
			:	m_items{ std::move( items ) }
			,	m_status_line_size{ 0 }
		{}

		/** @name Copy semantics.
		 * @brief Not allowed.
		*/
		///@{
		write_group_t( const write_group_t & ) = delete;
		write_group_t & operator = ( const write_group_t & ) = delete;
		///@}

		/** @name Move semantics.
		 * @brief Moves object leaving a moved one in clean state.
		*/
		///@{
		write_group_t( write_group_t && wg ) noexcept
			:	m_items{ std::move( wg.m_items ) }
			,	m_status_line_size{ wg.m_status_line_size }
			,	m_after_write_notificator{ std::move( wg.m_after_write_notificator ) }
		{
			wg.m_after_write_notificator = write_status_cb_t{}; // Make sure src is cleaned.
			wg.m_status_line_size = 0;
		}

		write_group_t & operator = ( write_group_t && wg ) noexcept
		{
			write_group_t tmp{ std::move( wg ) };
			swap( *this, tmp );

			return *this;
		}
		///@}

		//! Destruct object.
		/*!
			If notificator was not called it would be invoked with error.
		*/
		~write_group_t() noexcept
		{
			if( m_after_write_notificator )
			{
				restinio::utils::suppress_exceptions_quietly( [&] {
						invoke_after_write_notificator_if_exists(
							make_asio_compaible_error(
								asio_convertible_error_t::write_group_destroyed_passively ) );
					} );
			}
		}

		/** @name Auxiliary data.
		 * @brief Accessors for working with auxiliary data.
		*/
		///@{
		void
		status_line_size( std::size_t n )
		{
			if( std::size_t{0} != n )
			{
				if( m_items.empty() )
				{
					throw exception_t{
						"cannot set status line size for empty write group" };
				}

				if( writable_item_type_t::trivial_write_operation !=
					m_items.front().write_type() )
				{
					throw exception_t{
						"cannot set status line size for write group: "
						"first writable item must be 'trivial_write_operation'" };
				}

				if( m_items.front().size() < n )
				{
					throw exception_t{
						"cannot set status line size for write group: "
						"first writable item size is less than provided value" };
				}

				m_status_line_size = n;
			}
		}

		//! Get status line size.
		std::size_t
		status_line_size() const noexcept
		{
			return m_status_line_size;
		}

		//! Set after write notificator.
		void
		after_write_notificator( write_status_cb_t notificator ) noexcept
		{
			m_after_write_notificator = std::move( notificator );
		}

		//! Is there an after write notificator set?
		bool
		has_after_write_notificator() const noexcept
		{
			return static_cast< bool >( m_after_write_notificator );
		}

		//! Get after write notificator.
		void
		invoke_after_write_notificator_if_exists( const asio_ns::error_code & ec )
		{
			if( m_after_write_notificator )
			{
				auto tmp = std::move( m_after_write_notificator );

				// Make sure we clean notificator,
				// because on some platforms/compilers `std::move()` does not clean it.
				m_after_write_notificator = write_status_cb_t{};

				tmp( ec );
			}
		}
		///@}

		//! Get the count of stored items.
		auto
		items_count() const noexcept
		{
			return m_items.size();
		}

		//! Get access to the stored items.
		const auto &
		items() const noexcept
		{
			return m_items;
		}

		//! Get access to the stored items.
		/*!
			Should be used for cases where we should have a non-const
			access to writeable items.

			@since v.0.4.9
		*/
		auto &
		items() noexcept
		{
			return m_items;
		}

		//! Reset group.
		void
		reset() noexcept
		{

			RESTINIO_ENSURE_NOEXCEPT_CALL( m_items.clear() );
			m_status_line_size = 0;

			// This assign is expected to be noexcept.
			// And it is on some compilers.
			// But for some compilers std::function::operator= is not noexcept
			// (for example for Visual C++ from VisualStudio 2017).
			// So we have to hope that this assign won't throw.
			// Otherwise there is no way to recover from an exception
			// from std::function::operator= in that place.
			m_after_write_notificator = write_status_cb_t{};
		}

		//! Merges with another group.
		/*!
			Two groups can be merged if the first one has no after-write callback
			and the second one has no status line size.
		*/
		void
		merge( write_group_t second )
		{
			auto & second_items = second.m_items;
			m_items.reserve( m_items.size() + second_items.size() );

			std::move(
				begin( second_items ),
				end( second_items ),
				std::back_inserter( m_items ) );

			m_after_write_notificator = std::move( second.m_after_write_notificator );
		}

	private:
		//! A buffer objects included in this group.
		writable_items_container_t m_items;

		//! A size of status line located in first "buffer".
		/*!
			If the value is not 0 then it means it references
			a piece of data stored in the first buffer of m_items container.
		*/
		std::size_t m_status_line_size;

		//! A callback to invoke once the the write opertaion of a given group completes.
		write_status_cb_t m_after_write_notificator;
};

} /* namespace restinio */
