require 'mxx_ru/cpp'
MxxRu::Cpp::exe_target {

	if ENV.has_key?("RESTINIO_USES_BOOST_ASIO") and ENV["RESTINIO_USES_BOOST_ASIO"] == "1"
		# Add boost libs:
		lib_static( 'boost_system' )
	else
		required_prj 'asio_mxxru/prj.rb'
	end

	required_prj 'nodejs/http_parser_mxxru/prj.rb'
	required_prj 'fmt_mxxru/prj.rb'
	required_prj 'restinio/platform_specific_libs.rb'

	target( "_unit.test.handle_requests.output_and_buffers" )

	cpp_source( "const_buffer.cpp" )
	cpp_source( "std_string.cpp" )
	cpp_source( "shared_ptr_std_string.cpp" )
	cpp_source( "main.cpp" )
}

