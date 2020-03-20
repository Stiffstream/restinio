require 'mxx_ru/binary_unittest'

path = 'test/router/easy_parser_path_to_tuple'

Mxx_ru::setup_target(
	Mxx_ru::Binary_unittest_target.new(
		"#{path}/prj.ut.rb",
		"#{path}/prj.rb" )
)
