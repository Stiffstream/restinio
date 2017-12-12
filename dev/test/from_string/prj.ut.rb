require 'mxx_ru/binary_unittest'

Mxx_ru::setup_target(
	Mxx_ru::Binary_unittest_target.new(
		"test/from_string/prj.ut.rb",
		"test/from_string/prj.rb" )
)
