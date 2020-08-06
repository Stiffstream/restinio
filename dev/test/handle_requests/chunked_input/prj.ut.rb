require 'mxx_ru/binary_unittest'

path = 'test/handle_requests/chunked_input'

Mxx_ru::setup_target(
	Mxx_ru::Binary_unittest_target.new(
		"#{path}/prj.ut.rb",
		"#{path}/prj.rb" )
)
