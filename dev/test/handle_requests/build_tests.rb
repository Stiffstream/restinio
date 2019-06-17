#!/usr/bin/ruby
require 'mxx_ru/cpp'

MxxRu::Cpp::composite_target {

	%w[
		chunked_output
		echo_body
		method
		notificators
		output_and_buffers
		remote_endpoint
		connection_state
		ip_blocker
		slow_transmit
		throw_exception
		timeouts
		upgrade
		user_controlled_output
	].each do |name|
		required_prj "test/handle_requests/#{name}/prj.ut.rb"
	end
}
