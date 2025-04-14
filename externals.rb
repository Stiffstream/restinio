MxxRu::arch_externals :so5 do |e|
  e.url 'https://github.com/Stiffstream/sobjectizer/archive/v.5.8.1.1.tar.gz'

  e.map_dir 'dev/so_5' => 'dev'
end

MxxRu::arch_externals :asio do |e|
#  e.url 'https://github.com/chriskohlhoff/asio/archive/asio-1-18-0.tar.gz'
  e.url 'https://github.com/chriskohlhoff/asio/archive/asio-1-34-2.tar.gz'

  e.map_dir 'asio/include' => 'dev/asio'
end

MxxRu::arch_externals :llhttp do |e|
  e.url 'https://github.com/nodejs/llhttp/archive/refs/tags/release/v9.2.1.tar.gz'

  e.map_dir 'include' => 'dev/nodejs/llhttp'
  e.map_dir 'src' => 'dev/nodejs/llhttp'
  e.map_file 'CMakeLists.txt' => 'dev/nodejs/llhttp/*'
  e.map_file 'libllhttp.pc.in' => 'dev/nodejs/llhttp/*'
end

MxxRu::arch_externals :fmt do |e|
  e.url 'https://github.com/fmtlib/fmt/archive/11.1.4.zip'

  e.map_dir 'include' => 'dev/fmt'
  e.map_dir 'src' => 'dev/fmt'
  e.map_dir 'support' => 'dev/fmt'
  e.map_file 'CMakeLists.txt' => 'dev/fmt/*'
  e.map_file 'README.md' => 'dev/fmt/*'
  e.map_file 'ChangeLog.md' => 'dev/fmt/*'
end

MxxRu::arch_externals :expected_lite do |e|
  e.url 'https://github.com/martinmoene/expected-lite/archive/refs/tags/v0.8.0.tar.gz'

  e.map_dir 'include' => 'dev/expected-lite'
  e.map_dir 'cmake' => 'dev/expected-lite'
  e.map_file 'CMakeLists.txt' => 'dev/expected-lite/*'
  e.map_file 'LICENSE.txt' => 'dev/expected-lite/*'
  e.map_file 'README.md' => 'dev/expected-lite/*'
end

MxxRu::arch_externals :catch do |e|
  # NOTE: version 3.8.1 is not used because it leads to build errors
  # somewhere in catch2/extras/CatchAddTests.cmake:234
  # Something like:
  #
  # catch2/extras/CatchAddTests.cmake:138 (string):
  # string sub-command JSON failed parsing json string: * Line 1, Column 1
  #
  # Syntax error: value, object or array expected.
  e.url 'https://github.com/catchorg/Catch2/archive/refs/tags/v3.7.1.tar.gz'

  e.map_dir 'src' => 'dev/catch2/'
  e.map_dir 'CMake' => 'dev/catch2/'
  e.map_dir 'extras' => 'dev/catch2/'
  e.map_file 'CMakeLists.txt' => 'dev/catch2/*'
end

