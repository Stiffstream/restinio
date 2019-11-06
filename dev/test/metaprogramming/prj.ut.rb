require 'mxx_ru/binary_unittest'

Mxx_ru::setup_target(
	Mxx_ru::Binary_unittest_target.new(
		"test/metaprogramming/prj.ut.rb",
		"test/metaprogramming/prj.rb" )
)
