/*
 * RESTinio
 */

/*!
 * @file
 * @brief Stuff related to value of Cache-Control HTTP-field.
 *
 * @since v.0.6.1
 */

#pragma once

#include <restinio/helpers/http_field_parser.hpp>

namespace restinio
{

namespace http_field_parsers
{

//
// cache_control_value_t
//
struct cache_control_value_t
{
	using directive_container_t = std::map<
			std::string, restinio::optional_t<std::string> >;

	directive_container_t m_directives;

	struct details
	{
		template<typename T>
		struct container_adaptor {
			using container_type = directive_container_t;
			using value_type = restinio::optional_t<
					std::pair< std::string, restinio::optional_t<std::string> > >;

			static void
			store( container_type & dest, value_type && what )
			{
				if( what )
					dest.emplace( std::move(*what) );
			}
		};
	};

	static auto
	make_parser()
	{
		using namespace restinio::http_field_parser;

		using directive_t = std::pair<
				std::string,
				restinio::optional_t<std::string> >;

		return produce< cache_control_value_t >(
					repeat< nothing_t >( 0, N,
						symbol(',') >> skip(),
						rfc::ows() >> skip() ) >> skip(),

					produce< directive_t >(
						rfc::token() >> to_lower() >> &directive_t::first,
						optional< std::string >(
							symbol('=') >> skip(),
							alternatives< std::string >(
								rfc::token() >> to_lower(),
								rfc::quoted_string() ) >> as_result()
						) >> &directive_t::second
					) >> custom_consumer( [](auto & dest, directive_t && v) {
							dest.m_directives.emplace( std::move(v) );
						} ),

					repeat< directive_container_t, details::container_adaptor >(
						0, N,
						produce< restinio::optional_t<directive_t> >(
							rfc::ows() >> skip(),
							symbol(',') >> skip(),
							optional< directive_t >(
								rfc::ows() >> skip(),
								rfc::token() >> to_lower() >> &directive_t::first,
								optional< std::string >(
									symbol('=') >> skip(),
									alternatives< std::string >(
										rfc::token() >> to_lower(),
										rfc::quoted_string() ) >> as_result()
								) >> &directive_t::second
							) >> as_result()
						) >> as_result()
					) >> custom_consumer( [](auto & dest, directive_container_t && v) {
							// There is no a good way except the copy from
							// 'v' to 'dest' one-by-one.
							for( auto & item : v )
								dest.m_directives[ item.first ] = item.second;
						} )

				) >> as_result();
	}

	static std::pair< bool, cache_control_value_t >
	try_parse( string_view_t what )
	{
		return restinio::http_field_parser::try_parse_field_value< cache_control_value_t >(
				what,
				make_parser() );
	}
};

} /* namespace http_fields */

} /* namespace restinio */

