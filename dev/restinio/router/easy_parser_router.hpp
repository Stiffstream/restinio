/*
 * RESTinio
 */

//FIXME: make doxygen file header!

#pragma once

#include <restinio/router/impl/target_path_holder.hpp>
#include <restinio/router/non_matched_request_handler.hpp>

#include <restinio/helpers/easy_parser.hpp>

#include <vector>

namespace restinio
{

namespace router
{

namespace easy_parser_router
{

namespace impl
{

using target_path_holder_t = restinio::router::impl::target_path_holder_t;

struct no_match_t {};

class router_entry_t
{
public:
	virtual ~router_entry_t() = default;

	RESTINIO_NODISCARD
	virtual expected_t< request_handling_status_t, no_match_t >
	try_handle(
		const request_handle_t & req,
		target_path_holder_t & target_path ) const = 0;
};

using router_entry_unique_ptr_t = std::unique_ptr< router_entry_t >;

template< typename Producer, typename Handler >
class actual_router_entry_t : public router_entry_t
{
	http_method_id_t m_method;
	Producer m_producer;
	Handler m_handler;

public:
	template< typename Producer_Arg, typename Handler_Arg >
	actual_router_entry_t(
		http_method_id_t method,
		Producer_Arg && producer,
		Handler_Arg && handler )
		:	m_method{ method }
		,	m_producer{ std::forward<Producer_Arg>(producer) }
		,	m_handler{ std::forward<Handler_Arg>(handler) }
	{}

	RESTINIO_NODISCARD
	expected_t< request_handling_status_t, no_match_t >
	try_handle(
		const request_handle_t & req,
		target_path_holder_t & target_path ) const override
	{
		if( m_method == req->header().method() )
		{
			auto parse_result = easy_parser::try_parse(
					target_path.view(),
					m_producer );
			if( parse_result )
			{
				return m_handler( req, *parse_result );
			}
		}

		return make_unexpected( no_match_t{} );
	}
};

//
// exact_fragment_t
//
class exact_fragment_t
	:	public restinio::easy_parser::impl::producer_tag< bool >
{
	std::string m_fragment;

public:
	//FIXME: fragment shouldn't be empty.
	//This must be expressed somehow. Or must be checked at run-time.
	exact_fragment_t( std::string fragment )
		:	m_fragment{ std::move(fragment) }
	{}

	RESTINIO_NODISCARD
	expected_t< bool, restinio::easy_parser::parse_error_t >
	try_parse( restinio::easy_parser::impl::source_t & from )
	{
		restinio::easy_parser::impl::source_t::content_consumer_t consumer{ from };

		auto it = m_fragment.begin();
		const auto end = m_fragment.end();
		for( auto ch = from.getch(); !ch.m_eof; ch = from.getch() )
		{
			if( ch.m_ch != *it )
				return make_unexpected( restinio::easy_parser::parse_error_t{
						consumer.started_at(),
						restinio::easy_parser::error_reason_t::pattern_not_found
					} );
			if( ++it == end )
				break;
		}

		if( it != end )
			return make_unexpected( restinio::easy_parser::parse_error_t{
					consumer.started_at(),
					restinio::easy_parser::error_reason_t::unexpected_eof
				} );

		consumer.commit();

		return true;
	}
};

//
// unescape_transformer_t
//
template< typename Unescape_Traits >
struct unescape_transformer_t
	:	public restinio::easy_parser::impl::transformer_tag< std::string >
{
	using input_type = std::string;

	RESTINIO_NODISCARD
	result_type
	transform( input_type && input ) const
	{
		return restinio::utils::unescape_percent_encoding< Unescape_Traits >(
				input );
	}
};

} /* namespace impl */

using namespace restinio::easy_parser;

struct root_t {};

RESTINIO_NODISCARD
auto
root()
{
	return symbol_producer( '/' ) >> just( root_t{} );
}

RESTINIO_NODISCARD
auto
slash()
{
	return symbol( '/' );
}

RESTINIO_NODISCARD
auto
exact_producer( string_view_t fragment )
{
	return impl::exact_fragment_t{
			std::string{ fragment.data(), fragment.size() }
		};
}

RESTINIO_NODISCARD
auto
exact( string_view_t fragment )
{
	return impl::exact_fragment_t{
			std::string{ fragment.data(), fragment.size() }
		} >> skip();
}

RESTINIO_NODISCARD
auto
path_fragment( char separator = '/' )
{
	return produce< std::string >(
			repeat( 1, N,
					any_if_not_symbol_producer( separator ) >> to_container() ) );
}

template< typename Unescape_Traits =
		restinio::utils::restinio_default_unescape_traits >
RESTINIO_NODISCARD
auto
unescape()
{
	return impl::unescape_transformer_t< Unescape_Traits >{};
}

} /* namespace easy_parser_router */

class easy_parser_router_t
{
public:
	easy_parser_router_t() = default;

	easy_parser_router_t( const easy_parser_router_t & ) = delete;
	easy_parser_router_t &
	operator=( const easy_parser_router_t & ) = delete;

	easy_parser_router_t( easy_parser_router_t && ) = default;
	easy_parser_router_t &
	operator=( easy_parser_router_t && ) = default;

	RESTINIO_NODISCARD
	request_handling_status_t
	operator()( request_handle_t req ) const
	{
		using namespace easy_parser_router::impl;

		// Take care of an optional trailing slash.
		string_view_t path_to_inspect{ req->header().path() };
		if( path_to_inspect.size() > 1u && '/' == path_to_inspect.back() )
			path_to_inspect.remove_suffix( 1u );

		target_path_holder_t target_path{ path_to_inspect };
		for( const auto & entry : m_entries )
		{
			const auto r = entry->try_handle( req, target_path );
			if( r )
			{
				return *r;
			}
		}

		// Here: none of the routes matches this handler.
		if( m_non_matched_request_handler )
		{
			// If non matched request handler is set
			// then call it.
			return m_non_matched_request_handler( std::move( req ) );
		}

		return request_rejected();
	}

	template< typename Producer, typename Handler >
	void
	add_handler(
		http_method_id_t method,
		Producer && producer,
		Handler && handler )
	{
		using namespace easy_parser_router::impl;

		using producer_type = std::decay_t< Producer >;
		using handler_type = std::decay_t< Handler >;

		using actual_entry_type = actual_router_entry_t<
				producer_type, handler_type >;

		auto entry = std::make_unique< actual_entry_type >(
				method,
				std::forward<Producer>(producer),
				std::forward<Handler>(handler) );

		m_entries.push_back( std::move(entry) );
	}

	//! Set handler for requests that don't match any route.
	void
	non_matched_request_handler( non_matched_request_handler_t nmrh )
	{
		m_non_matched_request_handler= std::move( nmrh );
	}

private:
	using entries_container_t =
			std::vector< easy_parser_router::impl::router_entry_unique_ptr_t >;

	entries_container_t m_entries;

	//! Handler that is called for requests that don't match any route.
	non_matched_request_handler_t m_non_matched_request_handler;
};

} /* namespace router */

} /* namespace restinio */

