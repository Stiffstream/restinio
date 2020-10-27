/*
	restinio
*/

/*!
	Callbacks used with http parser.
*/

inline int
restinio_url_cb( http_parser * parser, const char * at, size_t length )
{
	try
	{
		auto * ctx =
			reinterpret_cast< restinio::impl::http_parser_ctx_t * >(
				parser->data );

		ctx->m_header.append_request_target( at, length );

		if( ctx->m_header.request_target().length() >
				ctx->m_limits.max_url_size() )
		{
			return -1;
		}
	}
	catch( const std::exception & )
	{
		return -1;
	}

	return 0;
}

inline int
restinio_header_field_cb( http_parser * parser, const char *at, size_t length )
{
	try
	{
		auto * ctx =
			reinterpret_cast< restinio::impl::http_parser_ctx_t * >(
				parser->data );

		if( ctx->m_last_was_value )
		{
			// Maybe there are too many fields?
			if( ctx->m_total_field_count == ctx->m_limits.max_field_count() )
			{
				return -1;
			}

			ctx->m_current_field_name.assign( at, length );
			ctx->m_last_was_value = false;
		}
		else
		{
			ctx->m_current_field_name.append( at, length );
		}

		if( ctx->m_current_field_name.size() >
				ctx->m_limits.max_field_name_size() )
		{
			return -1;
		}
	}
	catch( const std::exception & )
	{
		return -1;
	}

	return 0;
}

inline void
append_last_field_accessor( http_header_fields_t & fields, string_view_t value )
{
	fields.append_last_field( value );
}

inline int
restinio_header_value_cb( http_parser * parser, const char *at, size_t length )
{
	try
	{
		auto * ctx =
			reinterpret_cast< restinio::impl::http_parser_ctx_t * >( parser->data );

		http_header_fields_t & fields = ctx->m_leading_headers_completed
				? ctx->m_chunked_info_block.m_trailing_fields
				: ctx->m_header;

		if( !ctx->m_last_was_value )
		{
			fields.add_field(
				std::move( ctx->m_current_field_name ),
				std::string{ at, length } );

			ctx->m_last_value_total_size = length;
			ctx->m_last_was_value = true;

			// At this point the number of parsed fields can be incremented.
			ctx->m_total_field_count += 1u;
		}
		else
		{
			append_last_field_accessor( fields, std::string{ at, length } );
			ctx->m_last_value_total_size += length;
		}

		if( ctx->m_last_value_total_size >=
				ctx->m_limits.max_field_value_size() )
		{
			return -1;
		}
	}
	catch( const std::exception & )
	{
		return -1;
	}

	return 0;
}

inline int
restinio_headers_complete_cb( http_parser * parser )
{
	auto * ctx =
		reinterpret_cast< restinio::impl::http_parser_ctx_t * >(
			parser->data );
	// Next time header_name/header_value callback should store
	// values of trailing fields.
	ctx->m_leading_headers_completed = true;

	if( ULLONG_MAX != parser->content_length &&
		0 < parser->content_length )
	{
		// Maximum body size can be checked right now.
		if( parser->content_length > ctx->m_limits.max_body_size() )
		{
			return -1;
		}

		try
		{
			ctx->m_body.reserve(
					::restinio::utils::impl::uint64_to_size_t(
							parser->content_length) );
		}
		catch( const std::exception & )
		{
			return -1;
		}
	}

	return 0;
}


inline int
restinio_body_cb( http_parser * parser, const char *at, size_t length )
{
	try
	{
		auto * ctx =
			reinterpret_cast< restinio::impl::http_parser_ctx_t * >(
				parser->data );

		// The total size of the body should be checked.
		const auto total_length = static_cast<std::uint64_t>(
				ctx->m_body.size() ) + length;
		if( total_length > ctx->m_limits.max_body_size() )
		{
			return -1;
		}

		ctx->m_body.append( at, length );
	}
	catch( const std::exception & )
	{
		return -1;
	}

	return 0;
}

inline int
restinio_chunk_header_cb( http_parser * parser )
{
	try
	{
		// In on_chunk_header callback parser->content_length contains
		// the size of the next chunk.
		// If that size is 0 then it is the last chunk and it should be
		// ignored.
		if( 0u != parser->content_length )
		{
			auto * ctx =
				reinterpret_cast< restinio::impl::http_parser_ctx_t * >(
					parser->data );

			// Store an info about the new chunk.
			// If there will be an error at the next stage of parsing
			// the incoming request the whole request's data will be dropped.
			// So there is no need to care about that new item in m_chunks.
			ctx->m_chunked_info_block.m_chunks.emplace_back(
				ctx->m_body.size(),
				::restinio::utils::impl::uint64_to_size_t(parser->content_length) );
		}
	}
	catch( const std::exception & )
	{
		return -1;
	}

	return 0;
}

inline int
restinio_chunk_complete_cb( http_parser * /*parser*/ )
{
	// There is nothing to do.
	return 0;
}

template< typename Http_Methods >
int
restinio_message_complete_cb( http_parser * parser )
{
	// If entire http-message consumed, we need to stop parser.
	http_parser_pause( parser, 1 );

	auto * ctx =
		reinterpret_cast< restinio::impl::http_parser_ctx_t * >(
			parser->data );

	// Maybe the last trailing header is not handled yet.
	if( !ctx->m_last_was_value && !ctx->m_current_field_name.empty() )
	{
		ctx->m_chunked_info_block.m_trailing_fields.add_field(
				std::move( ctx->m_current_field_name ),
				std::string{} );
	}

	ctx->m_message_complete = true;
	ctx->m_header.method( Http_Methods::from_nodejs( parser->method ) );

	if( 0 == parser->upgrade )
		ctx->m_header.should_keep_alive( 0 != http_should_keep_alive( parser ) );
	else
		ctx->m_header.connection( http_connection_header_t::upgrade );

	return 0;
}
