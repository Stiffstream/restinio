require 'mxx_ru/binary_unittest'

Mxx_ru::setup_target(
	Mxx_ru::Binary_unittest_target.new(
		"test/encoders/prj.ut.rb",
		"test/encoders/prj.rb" )
)
