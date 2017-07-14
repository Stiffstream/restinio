require 'mxx_ru/cpp'

MxxRu::Cpp::exe_target {

  target 'sample.hello_world_https'

  required_prj 'asio_mxxru/prj.rb'
  required_prj 'nodejs/http_parser_mxxru/prj.rb'
  required_prj 'fmt_mxxru/prj.rb'
  required_prj 'sample/platform_specific_libs.rb'

  if 'mswin' == toolset.tag( 'target_os' )
    lib 'libeay32.lib'
    lib 'ssleay32.lib'
  else
    lib 'ssl'
    lib 'crypto'
  end

  cpp_source 'main.cpp'
}
