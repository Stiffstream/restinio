require 'mxx_ru/binary_unittest'

Mxx_ru::setup_target(
	Mxx_ru::Binary_unittest_target.new(
		"test/multipart_body/prj.ut.rb",
		"test/multipart_body/prj.rb" )
)
