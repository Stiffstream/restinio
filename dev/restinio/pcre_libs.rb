require 'mxx_ru/cpp'
require File.join( File.dirname(__FILE__), 'pcre_find.rb' )

MxxRu::Cpp::lib_collection_target {

  custom_local_pcre_prj = RestinioPCREFind.try_to_get_custom_file

  if File.exist?( custom_local_pcre_prj )
    required_prj custom_local_pcre_prj
  else
    define( "PCRE2_STATIC", Mxx_ru::Cpp::Target::OPT_UPSPREAD )
    RestinioPCREFind.get_libs( toolset ).each{|l| lib_static(l)}
  end
}

