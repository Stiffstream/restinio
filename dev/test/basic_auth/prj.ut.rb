require 'mxx_ru/binary_unittest'

Mxx_ru::setup_target(
	Mxx_ru::Binary_unittest_target.new(
		"test/basic_auth/prj.ut.rb",
		"test/basic_auth/prj.rb" )
)
