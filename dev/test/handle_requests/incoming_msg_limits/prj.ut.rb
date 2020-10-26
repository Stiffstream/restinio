require 'mxx_ru/binary_unittest'

path = 'test/handle_requests/incoming_msg_limits'

Mxx_ru::setup_target(
	Mxx_ru::Binary_unittest_target.new(
		"#{path}/prj.ut.rb",
		"#{path}/prj.rb" )
)
