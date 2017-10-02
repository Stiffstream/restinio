require 'mxx_ru/binary_unittest'

Mxx_ru::setup_target(
	Mxx_ru::Binary_unittest_target.new(
		"test/websocket/validators/prj.ut.rb",
		"test/websocket/validators/prj.rb" )
)
