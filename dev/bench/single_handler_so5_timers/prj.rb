require 'mxx_ru/cpp'

MxxRu::Cpp::exe_target {

  target '_bench.single_handler_so5_timers'

  required_prj 'asio_mxxru/prj.rb'
  required_prj 'nodejs/http_parser_mxxru/prj.rb'
  required_prj 'fmt_mxxru/prj.rb'
  required_prj 'rapidjson_mxxru/prj.rb'
  required_prj 'so_5/prj.rb'

  required_prj 'sample/platform_specific_libs.rb'

  cpp_source 'main.cpp'
}
