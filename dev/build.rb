#!/usr/bin/ruby
require 'mxx_ru/cpp'

MxxRu::Cpp::composite_target( MxxRu::BUILD_ROOT ) {

	toolset.force_cpp14
	global_include_path "."
	global_include_path "args"

	if not $sanitizer_build
		if 'gcc' == toolset.name || 'clang' == toolset.name
			global_linker_option '-pthread'
			global_linker_option '-static-libstdc++'
			global_linker_option "-Wl,-rpath='$ORIGIN'"
		end

		# If there is local options file then use it.
		if FileTest.exist?( "local-build.rb" )
			required_prj "local-build.rb"
		else
			default_runtime_mode( MxxRu::Cpp::RUNTIME_RELEASE )
			MxxRu::enable_show_brief

			global_obj_placement MxxRu::Cpp::PrjAwareRuntimeSubdirObjPlacement.new(
				'target', MxxRu::Cpp::PrjAwareRuntimeSubdirObjPlacement::USE_COMPILER_ID )
		end
	end

	required_prj 'test/build_tests.rb'
	required_prj 'sample/build_samples.rb'
}

