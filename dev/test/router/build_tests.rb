#!/usr/bin/ruby
require 'mxx_ru/cpp'
require 'restinio/pcre_find.rb'
require 'restinio/pcre2_find.rb'
require 'restinio/boost_helper.rb'

MxxRu::Cpp::composite_target {

	required_prj( "test/router/express/prj.ut.rb" )
	required_prj( "test/router/express_router/prj.ut.rb" )
	required_prj( "test/router/express_router_bench/prj.rb" )

	if RestinioPCREFind.has_pcre(toolset)
		required_prj( "test/router/express_pcre/prj.ut.rb" )
		required_prj( "test/router/express_router_pcre/prj.ut.rb" )
		required_prj( "test/router/express_router_pcre_bench/prj.rb" )
	end

	if RestinioPCRE2Find.has_pcre2(toolset)
		required_prj( "test/router/express_pcre2/prj.ut.rb" )
		required_prj( "test/router/express_router_pcre2/prj.ut.rb" )
		required_prj( "test/router/express_router_pcre2_bench/prj.rb" )
	end

	if RestinioBoostHelper.has_boost(toolset)
		required_prj( "test/router/express_boost_regex/prj.ut.rb" )
		required_prj( "test/router/express_router_boost_regex/prj.ut.rb" )
		required_prj( "test/router/express_router_boost_regex_bench/prj.rb" )
	end

	required_prj( "test/router/cmp_router_bench/prj.rb" )
}

