require 'mxx_ru/binary_unittest'

Mxx_ru::setup_target(
	Mxx_ru::Binary_unittest_target.new(
		"test/router/first_match_router/prj.ut.rb",
		"test/router/first_match_router/prj.rb" )
)
