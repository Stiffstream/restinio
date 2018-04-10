project(nodejs_http_parser)

add_library(nodejs_http_parser http_parser.c)
add_library(nodejs::http_parser ALIAS nodejs_http_parser)

target_include_directories(nodejs_http_parser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

