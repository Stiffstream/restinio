require 'mxx_ru/binary_unittest'

Mxx_ru::setup_target(
	Mxx_ru::Binary_unittest_target.new(
		"test/transforms/zlib_body_appender/prj.ut.rb",
		"test/transforms/zlib_body_appender/prj.rb" )
)
