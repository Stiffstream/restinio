set(SAMPLE sample.express_router)
include(${CMAKE_SOURCE_DIR}/cmake/sample.cmake)

target_include_directories(${SAMPLE} PRIVATE ${CMAKE_SOURCE_DIR}/rapidjson/include)
target_compile_definitions(${SAMPLE} PRIVATE -DRAPIDJSON_HAS_STDSTRING -DRAPIDJSON_HAS_CXX11_RVALUE_REFS)

