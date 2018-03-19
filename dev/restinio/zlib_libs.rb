require 'mxx_ru/cpp'
require File.join( File.dirname(__FILE__), 'zlib_find.rb' )

MxxRu::Cpp::lib_collection_target {

  system_zlib_libs = RestinioZlibFind.get_system_zlib( toolset )
  if system_zlib_libs
    system_zlib_libs.each{|l| lib(l)}
  else
    raise 'Own zlib sources build not implemented yet'
  end

}

