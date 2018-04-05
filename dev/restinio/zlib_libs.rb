require 'mxx_ru/cpp'
require File.join( File.dirname(__FILE__), 'zlib_find.rb' )

MxxRu::Cpp::lib_collection_target {

  if ENV.has_key?( "RESTINIO_USE_OWN_ZLIB_BUILD" )
    required_prj( 'restinio/third_party/zlib/prj.rb' )
  else
    system_zlib_libs = RestinioZlibFind.get_system_zlib( toolset )
    if system_zlib_libs
      system_zlib_libs.each{|l| lib(l)}
    else
      required_prj( 'restinio/third_party/zlib/prj.rb' )
    end
  end
}
