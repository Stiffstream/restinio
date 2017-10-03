require 'mxx_ru/binary_unittest'

Mxx_ru::setup_target(
	Mxx_ru::Binary_unittest_target.new(
		"test/socket_options_tls/prj.ut.rb",
		"test/socket_options_tls/prj.rb" )
)
