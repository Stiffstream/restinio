/*
	restinio
*/

/*!
	Selective include of asio/boost::asio.
*/

#pragma once

#if !defined(RESTINIO_USE_BOOST_ASIO)

// RESTinio uses stand-alone version of asio.
#include <asio.hpp>

// Define added to not have to distinguish between boost and non-boost asio in
// other code.
#define RESTINIO_ASIO_VERSION ASIO_VERSION

namespace restinio
{
	namespace asio_ns = ::asio;

	//! @name Adoptation functions to cover differences between snad-alone and beast asio.
	///@{
	inline bool
	error_is_operation_aborted( const asio_ns::error_code & ec ) noexcept
	{
		return ec == asio_ns::error::operation_aborted;
	}

	inline bool
	error_is_eof( const asio_ns::error_code & ec ) noexcept
	{
		return ec == asio_ns::error::eof;
	}
	///@}

	namespace asio_ec
	{
		constexpr auto  eof = asio_ns::error::eof;
		inline const auto & system_category() { return asio_ns::system_category(); }
	} /* namespace err */
	//! \}

	//! An alias for base class of error category entity.
	using error_category_base_t = asio_ns::error_category;

// Define a proxy macro for having the same name for asio stand-alone and boost.
#define RESTINIO_ERROR_CATEGORY_NAME_NOEXCEPT ASIO_ERROR_CATEGORY_NOEXCEPT

} /* namespace restinio */

	#if defined(ASIO_HAS_WINDOWS_OVERLAPPED_PTR)
		// Define feature macro with the same name for stand-alone and boost asio.
		#define RESTINIO_ASIO_HAS_WINDOWS_OVERLAPPED_PTR
	#endif

#else

// RESTinio uses boost::asio.
#include <boost/asio.hpp>

// Define added to not have to distinguish between boost and non-boost asio in
// other code.
#define RESTINIO_ASIO_VERSION BOOST_ASIO_VERSION

namespace restinio
{

	namespace asio_ns
	{
		using namespace ::boost::asio;
		using error_code = ::boost::system::error_code;
	} /* namespace asio_ns */

	//! @name Adoptation functions to cover differences between snad-alone and beast asio.
	///@{
	inline bool error_is_operation_aborted( const asio_ns::error_code & ec )
	{
		return ec == asio_ns::error::basic_errors::operation_aborted;
	}

	inline bool error_is_eof( const asio_ns::error_code & ec )
	{
		return ec == asio_ns::error::misc_errors::eof;
	}
	///@}

	namespace asio_ec
	{
		constexpr auto eof = asio_ns::error::misc_errors::eof;

		inline const auto & system_category() { return ::boost::system::system_category(); }

	} /* namespace err */

	//! An alias for base class of error category entity.
	using error_category_base_t = ::boost::system::error_category;

	// Define a proxy macro for having the same name for asio stand-alone and boost.
	#define RESTINIO_ERROR_CATEGORY_NAME_NOEXCEPT BOOST_SYSTEM_NOEXCEPT

} /* namespace restinio */

	#if defined(BOOST_ASIO_HAS_WINDOWS_OVERLAPPED_PTR)
		// Define feature macro with the same name for stand-alone and boost asio.
		#define RESTINIO_ASIO_HAS_WINDOWS_OVERLAPPED_PTR
	#endif

#endif

namespace restinio
{


//! Enum for restinio errors that must presented as asio_ns::error_code value.
/*
	@since v.0.4.8
*/
enum class asio_convertible_error_t : int
{
	// Notificators error.

	//! After write notificator error: data was not sent,
	//! connection closed (or aborted) before a given piece of data.
	write_was_not_executed = 100,

	//! After write notificator error: a notificator was set for a write_group_t
	//! but no external invokation happened, so write_group_t destructor
	//! calls it with error.
	write_group_destroyed_passively,

	//! A call to async_write failed.
	//! The corresponding write operation wasn't done.
	/*!
	 * @since v.0.6.0
	 */
	async_write_call_failed,

	//! A call to async_read_some_at failed.
	//! The corresponding sendfile operation wasn't done.
	/*!
	 * @since v.0.6.0
	 */
	async_read_some_at_call_failed
};

namespace impl
{

//! Error category for asio compatible error codes.
/*
	@since v.0.4.8
*/
class restinio_err_category_t : public error_category_base_t
{
	public:
		virtual const char*
		name() const RESTINIO_ERROR_CATEGORY_NAME_NOEXCEPT override
		{
			return "restinio";
		}

		virtual std::string
		message( int value ) const override
		{
			std::string result{};
			switch( static_cast< asio_convertible_error_t >( value ) )
			{
				case asio_convertible_error_t::write_was_not_executed:
					result.assign( "write operation was not" );
					break;
				case asio_convertible_error_t::write_group_destroyed_passively:
					result.assign(
						"write group destroyed without external notificato invokation" );
					break;
				case asio_convertible_error_t::async_write_call_failed:
					result.assign(
						"a call to async_write() failed" );
					break;
				case asio_convertible_error_t::async_read_some_at_call_failed:
					result.assign(
						"a call to async_read_some_at_call_failed() failed" );
					break;
			}

			return result;
		}
};

} /* namespace impl */

//! Get restinio error category.
/*
	@since v.0.4.8
*/
inline const error_category_base_t &
restinio_err_category()
{
	static impl::restinio_err_category_t instance;
	return instance;
}

//! Make restinio error_code compatible with asio_ns::error_code.
/*
	@since v.0.4.8
*/
inline asio_ns::error_code
make_asio_compaible_error( asio_convertible_error_t err ) noexcept
{
	return asio_ns::error_code{ static_cast< int >( err ), restinio_err_category() };
}

} /* namespace restinio */
