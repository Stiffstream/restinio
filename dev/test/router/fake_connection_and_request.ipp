#include "../common/fake_connection.ipp"

template< typename Extra_Data_Factory >
auto
create_fake_request(
	restinio::router::generic_easy_parser_router_t< Extra_Data_Factory > &,
	std::string target,
	http_method_id_t method = http_method_get() )
{
	using request_t = restinio::generic_request_t<
			typename Extra_Data_Factory::data_t
	>;

	Extra_Data_Factory extra_data_factory;
	return std::make_shared< request_t >(
			0,
			http_request_header_t{ method, std::move( target ) },
			"",
			std::make_shared< fake_connection_t >(),
			restinio::endpoint_t{
				restinio::asio_ns::ip::make_address_v4("127.0.0.1"),
				3000 },
			extra_data_factory );
}

