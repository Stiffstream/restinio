/*
 * RESTinio
 */

/*!
 * @file
 * @brief Stuff related to percent-encoded symbols.
 *
 * @since v.0.6.9
 */

#pragma once

#include <restinio/helpers/http_field_parsers/basics.hpp>

namespace restinio
{

namespace http_field_parsers
{

namespace details
{

//
// pct_encoded_result_type_t
//
/*!
 * @brief A type for representing extraction of percent-encoded char
 * from the input stream.
 *
 * Exactly three symbols are extracted: `% HEXDIGIT HEXDIGIT`
 *
 * @note
 * Moved into restinio::http_field_parsers::details namespace in v.0.6.9.
 *
 * @since v.0.6.1
 */
using pct_encoded_result_type_t = std::array< char, 3 >;

//
// pct_encoded_symbols_producer
//
/*!
 * @brief A producer that extract a sequence of symbols represented
 * a percent-encoded character.
 *
 * This producer returns instances of pct_encoded_result_type_t.
 *
 * @note
 * Moved into restinio::http_field_parsers::details namespace in v.0.6.9.
 *
 * @since v.0.6.1
 */
RESTINIO_NODISCARD
inline auto
pct_encoded_symbols_p()
{
	return produce< pct_encoded_result_type_t >(
			symbol_p( '%' ) >> to_container(),
			hexdigit_p() >> to_container(),
			hexdigit_p() >> to_container()
		);
}

//
// pct_encoded_symbols_consumer_t
//
/*!
 * @brief A special consumer that inserts an extracted sequence
 * of symbols into the result string.
 *
 * @note
 * Moved into restinio::http_field_parsers::details namespace in v.0.6.9.
 *
 * @since v.0.6.1
 */
struct pct_encoded_symbols_consumer_t
	: public restinio::easy_parser::impl::consumer_tag
{
	void
	consume( std::string & to, pct_encoded_result_type_t && from ) const
	{
		to.append( &from[0], from.size() );
	}
};

} /* namespace details */

} /* namespace http_field_parsers */

} /* namespace restinio */


