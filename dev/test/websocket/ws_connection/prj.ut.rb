require 'mxx_ru/binary_unittest'

Mxx_ru::setup_target(
	Mxx_ru::Binary_unittest_target.new(
		"test/websocket/ws_connection/prj.ut.rb",
		"test/websocket/ws_connection/prj.rb" )
)
