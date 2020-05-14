require 'rubygems'

gem 'Mxx_ru', '>= 1.3.0'

require 'mxx_ru/cpp'

MxxRu::Cpp::lib_target {

  # Define your target name here.
  target 'zlib-1.2.11'

  include_path './restinio/third_party/zlib', Mxx_ru::Cpp::Target::OPT_UPSPREAD

  if 'gcc' == toolset.name || 'clang' == toolset.name
    define( '_LARGEFILE64_SOURCE=1', Mxx_ru::Cpp::Target::OPT_UPSPREAD )
    define 'HAVE_HIDDEN'
  end

  if 'vc' == toolset.name
    define 'NO_FSEEKO'
    define '_CRT_SECURE_NO_DEPRECATE'
    define '_CRT_NONSTDC_NO_DEPRECATE'
  end

  # Enumerate your C/C++ files here.
  sources_root( '../zlib' ) {
    c_sources [
    'adler32.c',
    'crc32.c',
    'deflate.c',
    'infback.c',
    'inffast.c',
    'inflate.c',
    'inftrees.c',
    'trees.c',
    'zutil.c',
    'compress.c',
    'uncompr.c',
    'gzclose.c',
    'gzlib.c',
    'gzread.c',
    'gzwrite.c']
  }
}

