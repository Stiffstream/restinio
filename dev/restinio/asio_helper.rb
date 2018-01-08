# Helper functions for searching for external libs.
module RestinioAsioHelper
	@@msvc_libs_vc_tag = {
		'15'=>"vc141-mt",
		'14'=>"vc140-mt"
	}

	@@msvc_libs_dir_tag = {
		'15'=>"msvc-14.1",
		'14'=>"msvc-14.0"
	}

	@@boost_root = nil

	def self.detect_boost_root
		# Check BOOST_ROOT/BOOSTROOT env variables
		if @@boost_root.nil?
			if ENV.has_key? "BOOST_ROOT"
				@@boost_root = ENV[ "BOOST_ROOT" ]
			elsif ENV.has_key? "BOOSTROOT"
				@@boost_root = ENV[ "BOOSTROOT" ]
			else
				@@boost_root = ""
			end
		end

		@@boost_root
	end

	@@boost_ver = nil

	def self.detect_boost_ver
		if @@boost_ver.nil?
			if "" != self.detect_boost_root
				version_filename = File.join( self.detect_boost_root, "boost", "version.hpp" )
				bv = nil

				begin
					File.open( version_filename, "r") do |fin|
						fin.each_line do |line|
							if /#define\sBOOST_VERSION (?<bv>\d+)\s*$/ =~ line
								# //  BOOST_VERSION % 100 is the patch level
								# //  BOOST_VERSION / 100 % 1000 is the minor version
								# //  BOOST_VERSION / 100000 is the major version
								# #define BOOST_VERSION 106600 
								bv = bv.to_i
								break
							end
						end
					end
					if bv.nil?
						raise "unable to find BOOST_VERSION definition"
					end
				rescue => e 
					raise "unable to get boost version from #{version_filename}: #{e}"
				end

				if bv < 106600
					raise "boost version must be at least 106600"
				end

				version = [bv / 100000, bv / 100 % 1000 ]
				@@boost_ver = "#{version[0]}_#{version[1]}"
			else
				@@boost_ver = "1_66"
			end
		end
		
		@@boost_ver
	end

	def self.detect_bits( target_prj )
		if "x64" == target_prj.toolset.make_identification_string[-3..-1]
			bits = "64"
		else
			bits = "32"
		end
	end

	def self.get_msvc_name( lib_name, target_prj )
		# create name like:
		# boost_system-vc141-mt-gd-x64-1_66.lib
		# boost_system-vc141-mt-x64-1_66.lib
		# libboost_system-vc141-mt-gd-x64-1_66.lib
		# libboost_system-vc141-mt-sgd-x64-1_66.lib
		# libboost_system-vc141-mt-s-x64-1_66.lib
		# libboost_system-vc141-mt-x64-1_66.lib

		vc_ver = target_prj.toolset.tag( "ver_hi" )

		if not @@msvc_libs_vc_tag.has_key?( vc_ver )
			raise "current msvc toolset is not supported: must be vc14, vc15"
		end

		flags = ""

		if ENV["RESTINIO_USE_BOOST_ASIO"] == "shared"
			lib_name =  "#{lib_name}-#{@@msvc_libs_vc_tag[ vc_ver ]}"
		else
			lib_name =  "lib#{lib_name}-#{@@msvc_libs_vc_tag[ vc_ver ]}"

			if MxxRu::Cpp::RTL_STATIC == target_prj.mxx_rtl_mode
				flags += "s"
			end
		end

		if MxxRu::Cpp::RUNTIME_DEBUG == target_prj.mxx_runtime_mode
			flags += "gd"
		end

		if "" != flags
			lib_name += "-#{flags}"
		end

		"#{lib_name}-x#{self.detect_bits(target_prj)}-#{self.detect_boost_ver}"
	end
	
	# Attach project to a proper variant of asio (stand-alone or boost).
	def self.attach_propper_asio( target_prj )
		if ENV.has_key?("RESTINIO_USE_BOOST_ASIO")
			# Boost::ASIO must be used.
			if 'mswin' == target_prj.toolset.tag( 'target_os' )
				
				# Tricky naming on windows:

				libs = [self.get_msvc_name( 'boost_system', target_prj ) ]

				if self.detect_boost_root
					libdir = "lib#{self.detect_bits(target_prj)}-#{@@msvc_libs_dir_tag[target_prj.toolset.tag( "ver_hi" )]}"
					target_prj.lib_path( File.join( self.detect_boost_root, libdir ) )
					target_prj.include_path( self.detect_boost_root )
				end

				libs.each{|lib| target_prj.lib( lib ) }
			else
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
