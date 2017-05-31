require 'mxx_ru/cpp'

MxxRu::Cpp::composite_target {
	required_prj 'sample/hello_world_basic/prj.rb'
	required_prj 'sample/hello_world/prj.rb'
	required_prj 'sample/async_handling_with_sobjectizer/prj.rb'
	required_prj 'sample/express_router/prj.rb'
}
