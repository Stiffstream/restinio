#!/usr/bin/ruby
require 'mxx_ru/cpp'
require 'restinio/boost_helper.rb'

MxxRu::Cpp::composite_target( MxxRu::BUILD_ROOT ) {
  use_so_5_5 = 'so-5.5'

  if use_so_5_5 == ENV.fetch('RESTINIO_USE_LATEST_SO5', use_so_5_5)
    # SO-5.5 can be used with C++14.
    toolset.force_cpp14
  else
    # SObjectizer-5.7 requires C++17.
    toolset.force_cpp17
  end

  global_include_path "."
  global_include_path "clara" # It is necessary after merging
  # PR#47: https://github.com/Stiffstream/restinio/pull/47

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

  if "mswin" == toolset.tag( "target_os" ) && 'vc' == toolset.name && "" != RestinioBoostHelper.detect_boost_root
    RestinioBoostHelper.add_boost_root_path_msvc( self )
  end

  required_prj 'test/build_tests.rb'
  required_prj 'sample/build_samples.rb'
  required_prj 'benches/build_benches.rb'
}

