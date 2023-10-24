project(restinio_helpers CXX)

add_library(cmd_line_args INTERFACE)
add_library(restinio_helpers::cmd_line_args ALIAS cmd_line_args)

target_include_directories(cmd_line_args INTERFACE  ${CMAKE_CURRENT_LIST_DIR})

