#!/usr/bin/ruby
require 'mxx_ru/cpp'

MxxRu::Cpp::composite_target {
	required_prj "benches/single_handler/prj.rb"
	required_prj "benches/single_handler_so5_timer/prj.rb"
	required_prj "benches/single_handler_no_timer/prj.rb"
}
