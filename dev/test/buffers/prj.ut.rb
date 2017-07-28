require 'mxx_ru/binary_unittest'

Mxx_ru::setup_target(
	Mxx_ru::Binary_unittest_target.new(
		"test/buffers/prj.ut.rb",
		"test/buffers/prj.rb" )
)
