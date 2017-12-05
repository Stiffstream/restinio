require 'mxx_ru/cpp'
require File.join( File.dirname(__FILE__), 'pcre2_find.rb' )

MxxRu::Cpp::lib_collection_target {

  custom_local_pcre_prj = RestinioPCRE2Find.try_to_get_custom_file

  if File.exist?( custom_local_pcre_prj )
    required_prj custom_local_pcre_prj
  else
    define( "PCRE2_STATIC", Mxx_ru::Cpp::Target::OPT_UPSPREAD )
    define( "PCRE2_CODE_UNIT_WIDTH=8", Mxx_ru::Cpp::Target::OPT_UPSPREAD )
    RestinioPCRE2Find.get_libs( toolset ).each{|l| lib(l)}
  end
}

