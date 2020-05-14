require File.join( File.dirname(__FILE__), 'boost_helper.rb' )

# Helper functions for searching for external libs.
module RestinioAsioHelper


	def self.attach_boost_asio_msvc( target_prj )
		libs = [ RestinioBoostHelper.get_msvc_name( 'boost_system', target_prj ) ]
		libs.each{|lib| target_prj.lib( lib ) }
	end

	def self.attach_boost_asio_win_gcc( target_prj )
		libs = [ RestinioBoostHelper.get_gcc_name( 'boost_system', target_prj ) ]
		libs.each{|lib| target_prj.lib( lib ) }
	end

	# Attach project to a proper variant of asio (stand-alone or boost).
	def self.attach_propper_asio( target_prj )
		if ENV.has_key?("RESTINIO_USE_BOOST_ASIO")
			# Boost::ASIO must be used.
			bv = RestinioBoostHelper.detect_boost_ver( target_prj.toolset )
			if bv[1] < 106600
				raise "boost asio of version #{bv[0]} not supported"
			end

			if 'mswin' == target_prj.toolset.tag( 'target_os' )

				# Tricky naming on windows:

				if 'vc' == target_prj.toolset.name
					self.attach_boost_asio_msvc( target_prj )
				elsif 'gcc' == target_prj.toolset.name
					self.attach_boost_asio_win_gcc( target_prj )
				else
					raise "only vc/gcc toolsets are supported on windows"
				end
			else # Not mswin (linux)
				# Add boost libs:
				if ENV["RESTINIO_USE_BOOST_ASIO"] == "shared"
					target_prj.lib_shared( 'boost_system' )
				else
					target_prj.lib_static( 'boost_system' )
				end
			end
		else
			# Stand-alone ASIO must be used.
			target_prj.required_prj 'asio_mxxru/prj.rb'
		end
	end
end # module RestinioBoostAsioHelper
