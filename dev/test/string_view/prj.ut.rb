require 'mxx_ru/binary_unittest'

Mxx_ru::setup_target(
	Mxx_ru::Binary_unittest_target.new(
		"test/string_view/prj.ut.rb",
		"test/string_view/prj.rb" )
)
