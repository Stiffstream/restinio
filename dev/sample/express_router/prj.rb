require 'mxx_ru/cpp'

MxxRu::Cpp::exe_target {

  target 'sample.express_router'

  if ENV.has_key?("RESTINIO_USES_BOOST_ASIO") and ENV["RESTINIO_USES_BOOST_ASIO"] == "1"
    # Add boost libs:
    lib_static( 'boost_system' )
  else
    required_prj 'asio_mxxru/prj.rb'
  end

  required_prj 'nodejs/http_parser_mxxru/prj.rb'
  required_prj 'fmt_mxxru/prj.rb'
  required_prj 'rapidjson_mxxru/prj.rb'
  required_prj 'restinio/platform_specific_libs.rb'

  cpp_source 'main.cpp'
}
