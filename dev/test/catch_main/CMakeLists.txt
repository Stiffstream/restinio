add_library(restinio_catch_main STATIC catch_main.cpp)
add_library(restinio::catch_main ALIAS restinio_catch_main)

if (RESTINIO_FIND_DEPS)
    TARGET_LINK_LIBRARIES(restinio_catch_main Catch2::Catch2)
endif()

TARGET_INCLUDE_DIRECTORIES(restinio_catch_main PRIVATE ${CMAKE_SOURCE_DIR})
