require 'mxx_ru/cpp'
require File.join( File.dirname(__FILE__), 'openssl_find.rb' )

MxxRu::Cpp::lib_collection_target {

  custom_local_openssl_prj = RestinioOpenSSLFind.try_to_get_custom_file

  if File.exist?( custom_local_openssl_prj )
    required_prj custom_local_openssl_prj
  else
    RestinioOpenSSLFind.get_libs( toolset ).each{|l| lib(l)}
  end
}
