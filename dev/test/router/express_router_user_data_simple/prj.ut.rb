require 'mxx_ru/binary_unittest'

path = 'test/router/express_router_user_data_simple'

Mxx_ru::setup_target(
	Mxx_ru::Binary_unittest_target.new(
		"#{path}/prj.ut.rb",
		"#{path}/prj.rb" )
)
