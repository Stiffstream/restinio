require 'mxx_ru/binary_unittest'

Mxx_ru::setup_target(
	Mxx_ru::Binary_unittest_target.new(
		"test/try_parse_field/prj.ut.rb",
		"test/try_parse_field/prj.rb" )
)
