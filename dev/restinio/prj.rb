require 'mxx_ru/cpp'

MxxRu::Cpp::lib_target {

  target 'restinio'

  required_prj 'asio_mxxru/prj.rb'
  required_prj 'nodejs/http_parser_mxxru/prj.rb'
  required_prj 'fmt_mxxru/prj.rb'

  sources_root( 'impl' ) {
    cpp_source( 'connection.cpp' )
  }
}
