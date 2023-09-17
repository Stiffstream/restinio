MxxRu::arch_externals :so5 do |e|
  USE_SO_5_5 = 'so-5.5'
  if USE_SO_5_5 == ENV.fetch('RESTINIO_USE_LATEST_SO5', USE_SO_5_5)
    e.url 'https://github.com/eao197/so-5-5/archive/v5.5.24.4.tar.gz'

    e.map_dir 'dev/so_5' => 'dev'
    e.map_dir 'dev/timertt' => 'dev'
  else
    e.url 'https://github.com/Stiffstream/sobjectizer/archive/v.5.8.0.1.tar.gz'

    e.map_dir 'dev/so_5' => 'dev'
  end
end

MxxRu::arch_externals :asio do |e|
#  e.url 'https://github.com/chriskohlhoff/asio/archive/asio-1-18-0.tar.gz'
  e.url 'https://github.com/chriskohlhoff/asio/archive/asio-1-28-1.tar.gz'

  e.map_dir 'asio/include' => 'dev/asio'
end

MxxRu::arch_externals :llhttp do |e|
  e.url 'https://github.com/nodejs/llhttp/archive/refs/tags/release/v9.1.2.tar.gz'

  e.map_dir 'include' => 'dev/nodejs/llhttp'
  e.map_dir 'src' => 'dev/nodejs/llhttp'
end

MxxRu::arch_externals :fmt do |e|
  e.url 'https://github.com/fmtlib/fmt/archive/10.1.0.zip'
#  e.url 'https://github.com/fmtlib/fmt/archive/8.1.1.zip'
#  e.url 'https://github.com/fmtlib/fmt/archive/7.1.3.zip'

  e.map_dir 'include' => 'dev/fmt'
  e.map_dir 'src' => 'dev/fmt'
  e.map_dir 'support' => 'dev/fmt'
  e.map_file 'CMakeLists.txt' => 'dev/fmt/*'
  e.map_file 'README.rst' => 'dev/fmt/*'
  e.map_file 'ChangeLog.rst' => 'dev/fmt/*'
end

MxxRu::arch_externals :expected_lite do |e|
  e.url 'https://github.com/martinmoene/expected-lite/archive/refs/tags/v0.6.3.tar.gz'

  e.map_dir 'include' => 'dev/expected-lite'
end

MxxRu::arch_externals :rapidjson do |e|
  e.url 'https://zlib.net/zlib-1.3.tar.gz'

  e.map_dir 'include/rapidjson' => 'dev/rapidjson/include'
end

MxxRu::arch_externals :rapidjson do |e|
  e.url 'https://github.com/miloyip/rapidjson/archive/v1.1.0.zip'

  e.map_dir 'include/rapidjson' => 'dev/rapidjson/include'
end

MxxRu::arch_externals :json_dto do |e|
  e.url 'https://github.com/Stiffstream/json_dto/archive/v.0.2.15.tar.gz'

  e.map_dir 'dev/json_dto' => 'dev'
end

MxxRu::arch_externals :clara do |e|
  e.url 'https://github.com/catchorg/Clara/archive/v1.1.5.tar.gz'

  e.map_file 'single_include/clara.hpp' => 'dev/clara/*'
end

MxxRu::arch_externals :catch do |e|
  e.url 'https://github.com/catchorg/Catch2/archive/v2.13.9.tar.gz'

  e.map_dir 'single_include/catch2' => 'dev'
end

