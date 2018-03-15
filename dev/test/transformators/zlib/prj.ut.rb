require 'mxx_ru/binary_unittest'

Mxx_ru::setup_target(
	Mxx_ru::Binary_unittest_target.new(
		"test/transformators/zlib/prj.ut.rb",
		"test/transformators/zlib/prj.rb" )
)
