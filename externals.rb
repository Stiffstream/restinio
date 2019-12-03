MxxRu::arch_externals :so5 do |e|
  e.url 'https://github.com/eao197/so-5-5/archive/v5.5.24.4.tar.gz'

  e.map_dir 'dev/so_5' => 'dev'
  e.map_dir 'dev/timertt' => 'dev'
end

MxxRu::arch_externals :asio do |e|
  e.url 'https://github.com/chriskohlhoff/asio/archive/asio-1-14-0.tar.gz'

  e.map_dir 'asio/include' => 'dev/asio'
end

MxxRu::arch_externals :asio_mxxru do |e|
  e.url 'https://github.com/Stiffstream/asio_mxxru/archive/1.1.2.tar.gz'

  e.map_dir 'dev/asio_mxxru' => 'dev'
end

MxxRu::arch_externals :nodejs_http_parser do |e|
  e.url 'https://github.com/nodejs/http-parser/archive/v2.9.2.tar.gz'

  e.map_file 'http_parser.h' => 'dev/nodejs/http_parser/*'
  e.map_file 'http_parser.c' => 'dev/nodejs/http_parser/*'
end

MxxRu::arch_externals :nodejs_http_parser_mxxru do |e|
  e.url 'https://github.com/Stiffstream/nodejs_http_parser_mxxru/archive/v.0.2.1.tar.gz'

  e.map_dir 'dev/nodejs/http_parser_mxxru' => 'dev/nodejs'
end

MxxRu::arch_externals :fmt do |e|
  e.url 'https://github.com/fmtlib/fmt/archive/6.1.0.zip'

  e.map_dir 'include' => 'dev/fmt'
  e.map_dir 'src' => 'dev/fmt'
  e.map_dir 'support' => 'dev/fmt'
  e.map_file 'CMakeLists.txt' => 'dev/fmt/*'
  e.map_file 'README.rst' => 'dev/fmt/*'
  e.map_file 'ChangeLog.rst' => 'dev/fmt/*'
end

MxxRu::arch_externals :fmtlib_mxxru do |e|
  e.url 'https://github.com/Stiffstream/fmtlib_mxxru/archive/fmt-5.0.0-1.tar.gz'

  e.map_dir 'dev/fmt_mxxru' => 'dev'
end

MxxRu::arch_externals :rapidjson do |e|
  e.url 'https://github.com/miloyip/rapidjson/archive/v1.1.0.zip'

  e.map_dir 'include/rapidjson' => 'dev/rapidjson/include'
end

MxxRu::arch_externals :rapidjson_mxxru do |e|
  e.url 'https://github.com/Stiffstream/rapidjson_mxxru/archive/v.1.0.1.tar.gz'

  e.map_dir 'dev/rapidjson_mxxru' => 'dev'
end

MxxRu::arch_externals :json_dto do |e|
  e.url 'https://github.com/Stiffstream/json_dto/archive/v.0.2.9.2.tar.gz'

  e.map_dir 'dev/json_dto' => 'dev'
end

MxxRu::arch_externals :clara do |e|
  e.url 'https://github.com/catchorg/Clara/archive/v1.1.5.tar.gz'

  e.map_file 'single_include/clara.hpp' => 'dev/clara/*'
end

MxxRu::arch_externals :catch do |e|
  e.url 'https://github.com/catchorg/Catch2/archive/v2.11.0.tar.gz'

  e.map_dir 'single_include/catch2' => 'dev'
end

