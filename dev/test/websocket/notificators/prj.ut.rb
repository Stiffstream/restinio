require 'mxx_ru/binary_unittest'

Mxx_ru::setup_target(
	Mxx_ru::Binary_unittest_target.new(
		"test/websocket/notificators/prj.ut.rb",
		"test/websocket/notificators/prj.rb" )
)
