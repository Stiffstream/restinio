#!/usr/bin/ruby
require 'mxx_ru/cpp'

MxxRu::Cpp::composite_target {

	%w[
		zlib
		zlib_body_appender
		zlib_body_handler
	].each do |name|
		required_prj "test/transforms/#{name}/prj.ut.rb"
	end
}
