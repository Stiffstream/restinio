require 'mxx_ru/binary_unittest'

Mxx_ru::setup_target(
	Mxx_ru::Binary_unittest_target.new(
		"test/http_pipelining/sequence/prj.ut.rb",
		"test/http_pipelining/sequence/prj.rb" )
)
