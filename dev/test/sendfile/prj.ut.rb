require 'mxx_ru/binary_unittest'

Mxx_ru::setup_target(
	Mxx_ru::Binary_unittest_target.new(
		"test/sendfile/prj.ut.rb",
		"test/sendfile/prj.rb" )
)
