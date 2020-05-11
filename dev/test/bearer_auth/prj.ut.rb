require 'mxx_ru/binary_unittest'

Mxx_ru::setup_target(
	Mxx_ru::Binary_unittest_target.new(
		"test/bearer_auth/prj.ut.rb",
		"test/bearer_auth/prj.rb" )
)
