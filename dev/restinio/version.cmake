file(READ "${CMAKE_CURRENT_LIST_DIR}/version.hpp" version_hpp)

string(REGEX MATCH "RESTINIO_VERSION_MAJOR ([0-9]+)ull" MATCHED_CONTENT ${version_hpp})

if(MATCHED_CONTENT)
    set(RESTINIO_VERSION_MAJOR ${CMAKE_MATCH_1})
else()
    message(FATAL_ERROR "Failed to extract RESTINIO_VERSION_MAJOR from version.hpp")
endif()

string(REGEX MATCH "RESTINIO_VERSION_MINOR ([0-9]+)ull" MATCHED_CONTENT ${version_hpp})

if(MATCHED_CONTENT)
    set(RESTINIO_VERSION_MINOR ${CMAKE_MATCH_1})
else()
    message(FATAL_ERROR "Failed to extract RESTINIO_VERSION_MINOR from version.hpp")
endif()

string(REGEX MATCH "RESTINIO_VERSION_PATCH ([0-9]+)ull" MATCHED_CONTENT ${version_hpp})

if(MATCHED_CONTENT)
    set(RESTINIO_VERSION_PATCH ${CMAKE_MATCH_1})
else()
    message(FATAL_ERROR "Failed to extract RESTINIO_VERSION_PATCH from version.hpp")
endif()


set(RESTINIO_VERSION
    ${RESTINIO_VERSION_MAJOR}.${RESTINIO_VERSION_MINOR}.${RESTINIO_VERSION_PATCH})

set(RESTINIO_LIB_SOVERSION
    ${RESTINIO_VERSION_MAJOR}.${RESTINIO_VERSION_MINOR})

message(STATUS "RESTINIO_VERSION:   ${RESTINIO_VERSION}")
message(STATUS "RESTINIO_SOVERSION: ${RESTINIO_LIB_SOVERSION}")
