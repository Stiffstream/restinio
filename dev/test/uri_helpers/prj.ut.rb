require 'mxx_ru/binary_unittest'

Mxx_ru::setup_target(
	Mxx_ru::Binary_unittest_target.new(
		"test/uri_helpers/prj.ut.rb",
		"test/uri_helpers/prj.rb" )
)
