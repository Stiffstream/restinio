require 'mxx_ru/cpp'

MxxRu::Cpp::exe_target {

  target 'sample.async_handling_with_sobjectizer'

  required_prj 'asio_mxxru/prj.rb'
  required_prj 'nodejs/http_parser_mxxru/prj.rb'
  required_prj 'fmt_mxxru/prj.rb'
  required_prj 'restinio/platform_specific_libs.rb'
  required_prj 'so_5/prj.rb'

  cpp_source 'main.cpp'
}
