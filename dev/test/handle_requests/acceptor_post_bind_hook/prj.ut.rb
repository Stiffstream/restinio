require 'mxx_ru/binary_unittest'

path = 'test/handle_requests/acceptor_post_bind_hook'

Mxx_ru::setup_target(
	Mxx_ru::Binary_unittest_target.new(
		"#{path}/prj.ut.rb",
		"#{path}/prj.rb" )
)
