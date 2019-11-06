require 'mxx_ru/binary_unittest'

Mxx_ru::setup_target(
	Mxx_ru::Binary_unittest_target.new(
		"test/file_upload/prj.ut.rb",
		"test/file_upload/prj.rb" )
)
