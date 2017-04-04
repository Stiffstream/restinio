require 'mxx_ru/binary_unittest'

Mxx_ru::setup_target(
	Mxx_ru::Binary_unittest_target.new(
		"test/handle_requests/throw_exception/prj.ut.rb",
		"test/handle_requests/throw_exception/prj.rb" )
)
