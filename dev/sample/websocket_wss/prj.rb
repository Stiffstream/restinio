require 'mxx_ru/cpp'
require 'restinio/asio_helper.rb'

MxxRu::Cpp::exe_target {

  target 'sample.websocket_wss'

  RestinioAsioHelper.attach_propper_asio( self )

  required_prj 'nodejs/http_parser_mxxru/prj.rb'
  required_prj 'fmt_mxxru/prj.rb'
  required_prj 'restinio/platform_specific_libs.rb'
  required_prj 'restinio/open_ssl_libs.rb'

  cpp_source 'main.cpp'
}
