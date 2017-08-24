require 'mxx_ru/cpp'

MxxRu::Cpp::exe_target {

	target( "_bench.test.to_lower_bench" )

	cpp_source( "main.cpp" )
}

