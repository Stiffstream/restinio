require 'mxx_ru/binary_unittest'

Mxx_ru::setup_target(
	Mxx_ru::Binary_unittest_target.new(
		"test/start_stop/prj.ut.rb",
		"test/start_stop/prj.rb" )
)
