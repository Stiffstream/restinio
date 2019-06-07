require 'mxx_ru/binary_unittest'

Mxx_ru::setup_target(
	Mxx_ru::Binary_unittest_target.new(
		"test/handle_requests/connection_state/prj.ut.rb",
		"test/handle_requests/connection_state/prj.rb" )
)
