require 'mxx_ru/binary_unittest'

Mxx_ru::setup_target(
	Mxx_ru::Binary_unittest_target.new(
		"test/router/express_router_pcre2/prj.ut.rb",
		"test/router/express_router_pcre2/prj.rb" )
)
