require 'mxx_ru/cpp'

MxxRu::Cpp::lib_collection_target {
  if 'mswin' == toolset.tag( 'target_os' )
    lib 'wsock32' 
    lib 'ws2_32' 
  end
}
