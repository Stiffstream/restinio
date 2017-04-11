require 'mxx_ru/binary_unittest'

Mxx_ru::setup_target(
	Mxx_ru::Binary_unittest_target.new(
		"test/response_coordinator/prj.ut.rb",
		"test/response_coordinator/prj.rb" )
)
