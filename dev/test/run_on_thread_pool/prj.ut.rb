require 'mxx_ru/binary_unittest'

Mxx_ru::setup_target(
	Mxx_ru::Binary_unittest_target.new(
		"test/run_on_thread_pool/prj.ut.rb",
		"test/run_on_thread_pool/prj.rb" )
)
