require 'mxx_ru/cpp'

MxxRu::Cpp::composite_target {
	required_prj 'bench/single_handler/prj.rb'
	required_prj 'bench/single_handler_no_timer/prj.rb'
	required_prj 'bench/single_handler_so5_timers/prj.rb'
}
