require File.join( File.dirname(__FILE__), 'lib_finder.rb' )

module RestinioBoostHelper
	@@msvc_libs_vc_tag = {
		'16'=>"vc142-mt",
		'15'=>"vc141-mt",
		'14'=>"vc140-mt"
	}

	@@msvc_libs_dir_tag = {
		'16'=>"msvc-14.2",
		'15'=>"msvc-14.1",
		'14'=>"msvc-14.0"
	}


	def self.read_version_from_version_hpp( fpath )
		bv = nil
		begin
			File.open( fpath, "r") do |fin|
				fin.each_line do |line|
					if /#define\sBOOST_VERSION (?<bv>\d+)\s*$/ =~ line
						# #define BOOST_VERSION 106600
						bv = bv.to_i
						break
					end
				end
			end
			if bv.nil?
				raise "unable to find BOOST_VERSION definition in #{fpath}"
			end
		rescue => e
			raise "unable to get boost version from #{fpath}: #{e}"
		end

		bv
	end

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

	def self.add_boost_root_path_msvc( target_prj )
		libdir = "lib#{detect_bits(target_prj)}-#{@@msvc_libs_dir_tag[target_prj.toolset.tag( "ver_hi" )]}"
		target_prj.global_linker_option( "/LIBPATH:\"#{File.join( detect_boost_root, libdir )}\"" )
		target_prj.global_include_path( detect_boost_root )
	end

	def self.get_include_dirs_msvc
		if ENV[ "INCLUDE" ]
			ENV[ "INCLUDE" ].split(";")
		else
			[]
		end
	end

	def self.get_include_dirs_gcc( toolset )
		File.open( 'fake_cpp_file_for_detect_include_dir.cpp', "w" ){}

		dirs = []
		IO.popen( "#{toolset.cpp_compiler_name} -E -x c++ - -v 2>&1 < fake_cpp_file_for_detect_include_dir.cpp", :err => [:child, :out] ) do |io|
			collect_dirs = false
			io.each_line do |line|
				if /\#include \<\.\.\.\> search starts here\:/ =~ line
					collect_dirs = true
				elsif /End of search list\./ =~ line
					collect_dirs = false
				elsif collect_dirs
					dirs << line.strip
				end
			end
		end

		File.delete( 'fake_cpp_file_for_detect_include_dir.cpp' )

		dirs
	end

	def self.find_boost_ver_based_on_include_dirs( toolset )
		# Check BOOST_ROOT/BOOSTROOT env variables
		bv = nil
		include_dirs = []
		if "vc" == toolset.name
			include_dirs = self.get_include_dirs_msvc
		elsif "gcc" == toolset.name
			include_dirs = self.get_include_dirs_gcc( toolset )
		end

		include_dirs.each do |d|
			version_filename = File.join( d, "boost/version.hpp" )
			if File.exists?( version_filename )
				bv = read_version_from_version_hpp( version_filename )
				@@boost_in_include_dirs=d
				break;
			end
		end

		if bv.nil?
			puts "include dirs:\n  " + include_dirs.join( "\n  " )
			raise "Boost not found in inclide directories: " +
				"based on search for <include_dir>/boost/version.hpp\n"
		end

		bv
	end

	@@bv = nil
	@@boost_ver = nil

	def self.detect_boost_ver( toolset )
		if @@boost_ver.nil?
			if "" != self.detect_boost_root
				version_filename = File.join( self.detect_boost_root, "boost", "version.hpp" )
				@@bv = read_version_from_version_hpp( version_filename )
			else
				@@bv = self.find_boost_ver_based_on_include_dirs( toolset )
			end

			# //  BOOST_VERSION % 100 is the patch level
			# //  BOOST_VERSION / 100 % 1000 is the minor version
			# //  BOOST_VERSION / 100000 is the major version
			@@boost_ver = "#{@@bv / 100000}_#{@@bv / 100 % 1000}"
			if 0 != @@bv % 100
				@@boost_ver += "_#{@@bv % 100}"
			end
		end

		[@@boost_ver, @@bv ]
	end

	@has_boost = nil
	def self.has_boost( toolset )
		if @has_boost.nil?
			begin
				x = detect_boost_ver( toolset )
				p x
				@has_boost = true
			rescue
				@has_boost = false
			end
		end
		@has_boost
	end


	def self.detect_bits( target_prj )
		bits = "32"

		if "vc" == target_prj.toolset.name
			if "x64" == target_prj.toolset.make_identification_string[-3..-1]
				bits = "64"
			end
		else
			if target_prj.toolset.make_identification_string.include? "x86_64"
				bits = "64"
			end
		end

		bits
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
			raise "current msvc toolset is not supported: must be vc14, vc15, vc16"
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

		"#{lib_name}-x#{self.detect_bits(target_prj)}-#{self.detect_boost_ver(target_prj.toolset)[0]}"
	end

	@@gcc_version_tag = nil
	def self.detect_gcc_version_tag( toolset )
		if @@gcc_version_tag.nil?
			IO.popen( "#{toolset.cpp_compiler_name} -v 2>&1", :err => [:child, :out] ) do |io|
				io.each_line do |line|
					if /^gcc version (?<v1>\d+)\.(?<v2>\d+)/ =~ line
						@@gcc_version_tag = "mgw#{v1}#{v2}"
					end
				end
			end
			if @@gcc_version_tag.nil?
				raise "unable to detect gcc version"
			end
		end
		@@gcc_version_tag
	end

	def self.get_gcc_name( lib_name, target_prj )
		# create name like:
		# boost_system-mgw71-mt-d-x32-1_66.a
		# boost_system-mgw71-mt-d-x64-1_66.a
		# boost_system-mgw71-mt-d-x64-1_66.dll.a
		# boost_system-mgw71-mt-sd-x32-1_66.a
		# boost_system-mgw71-mt-sd-x64-1_66.a
		# boost_system-mgw71-mt-s-x32-1_66.a
		# boost_system-mgw71-mt-s-x64-1_66.a
		# boost_system-mgw71-mt-x32-1_66.a
		# boost_system-mgw71-mt-x64-1_66.a
		# boost_system-mgw71-mt-x64-1_66.dll.a

		gcc_ver = detect_gcc_version_tag( target_prj.toolset )

		lib_name =  "#{lib_name}-#{gcc_ver}-mt"
		flags = ""

		if ENV["RESTINIO_USE_BOOST_ASIO"] == "static" and MxxRu::Cpp::RTL_STATIC == target_prj.mxx_rtl_mode
			flags += "s"
		end

		if MxxRu::Cpp::RUNTIME_DEBUG == target_prj.mxx_runtime_mode
			flags += "d"
		end

		if "" != flags
			lib_name += "-#{flags}"
		end

		lib_name = "#{lib_name}-x#{self.detect_bits(target_prj)}-#{self.detect_boost_ver(target_prj.toolset)[0]}"

		if ENV["RESTINIO_USE_BOOST_ASIO"] == "shared"
			lib_name+= ".dll"
		end

		lib_name
	end

	def self.get_lib_name( lib_name, target_prj )
		if 'mswin' == target_prj.toolset.tag( 'target_os' )
			if 'vc' == target_prj.toolset.name
				self.get_msvc_name( lib_name, target_prj )
			elsif 'gcc' == target_prj.toolset.name
				self.get_gcc_name( lib_name, target_prj )
			end
		else
			lib_name
		end
	end

end # module RestinioBoostHelper
