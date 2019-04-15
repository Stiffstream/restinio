set(SAMPLE sample.express_router_tutorial)
include(${CMAKE_SOURCE_DIR}/cmake/sample.cmake)

#FIXME: for test only!
target_compile_options(${SAMPLE} PRIVATE "/permissive-")
