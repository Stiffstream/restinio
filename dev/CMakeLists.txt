# RESTinio CMake Project File
#
# This CMake file defines no targets on its own. It serves the following purposes:
#   * Setting compiler flags.
#   * Resolving dependencies (e.g., find_package for necessary libraries).
#   * Gathering meaningful targets from subdirectories (add_subdirectory).
#
# This file serves as the primary build entry for RESTinio's developers
#  and is not intended for use when RESTinio is added as a dependency
#  in another project.
#
# If you are not using a package manager (such as Conan or Vcpkg) and wish to use
# RESTinio as a submodule or any other approach that involves downloading a tarball
# of sources and building RESTinio as part of your CMake-based project,
# consider using the CMake file that defines RESTinio targets
# (found in <prj_root>/dev/destionio). That will simplify the process
# of incorporating RESTinio into your project, avoiding the need to disable
# various build environment settings defined in this file.


# Root CMake for logr project.
cmake_minimum_required(VERSION 3.8)

set(RESTINIO_LIBRARY_NAME restinio)
project(mvd${RESTINIO_LIBRARY_NAME}_root CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if (RESTINIO_EXPLICIT_CPPSTD)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/cpp_standard.cmake)
    # The C++ standard is set explicit by user.
    if ("${RESTINIO_EXPLICIT_CPPSTD}" STREQUAL "17" )
        message(STATUS "ENABLE C++17")
        cxx_17()
    elseif ("${RESTINIO_EXPLICIT_CPPSTD}" STREQUAL "20")
        message(STATUS "ENABLE C++20")
        cxx_20()
    elseif ("${RESTINIO_EXPLICIT_CPPSTD}" STREQUAL "23")
        message(STATUS "ENABLE C++23")
        cxx_23()
    else ()
        message(FATAL_ERROR
                "invalid RESTINIO_EXPLICIT_CPPSTD (value ='${RESTINIO_EXPLICIT_CPPSTD}')"
                "must be on of: [17, 20, 23]")
    endif ()
endif()

if (RESTINIO_EXPLICIT_LIBCXX)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/libcxx.cmake)
    handle_explicit_libcxx_if_necessary(${RESTINIO_EXPLICIT_LIBCXX})
endif()

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/link_threads_if_necessary.cmake")

option(RESTINIO_INSTALL "Generate the install target." ON)
option(RESTINIO_TEST "Build the tests." ON)
option(RESTINIO_SAMPLE "Build samples." ON)
option(RESTINIO_INSTALL_SAMPLES "Build install samples." ON)
option(RESTINIO_BENCHMARK "Build the benchmarks." ON)
option(RESTINIO_INSTALL_BENCHMARK "Install benchmarks." ON)
option(RESTINIO_WITH_SOBJECTIZER "Add RESTinio sobjectizer integration" ON)

message(STATUS "RESTINIO_INSTALL:           ${RESTINIO_INSTALL}")
message(STATUS "RESTINIO_TEST:              ${RESTINIO_TEST}")
message(STATUS "RESTINIO_SAMPLE:            ${RESTINIO_SAMPLE}")
message(STATUS "RESTINIO_INSTALL_SAMPLES:   ${RESTINIO_INSTALL_SAMPLES}")
message(STATUS "RESTINIO_BENCHMARK:         ${RESTINIO_BENCHMARK}")
message(STATUS "RESTINIO_INSTALL_BENCHMARK: ${RESTINIO_INSTALL_BENCHMARK}")
message(STATUS "RESTINIO_WITH_SOBJECTIZER:  ${RESTINIO_WITH_SOBJECTIZER}")

# By default we assume to use an embedded version of ASIO.
option(RESTINIO_USE_SYSTEM_STANDALONE_ASIO "Assume ASIO is available as system headers" OFF)

# By default we assume to use Boost::asio other than system.
# to be more specific we will assume that find_package would do it.
option(RESTINIO_USE_SYSTEM_BOOST_ASIO "Assume Boosr::asio is available as system headers" OFF)

# By default we assume to use an embedded version of expected-lite.
option(RESTINIO_USE_SYSTEM_LLHTTP "Assume llhttp (http parser) is available as system library" OFF)

# By default we assume to use an embedded version of expected-lite.
option(RESTINIO_USE_SYSTEM_FMT "Assume FMT is available as system headers" OFF)

# By default we assume to use an embedded version of expected-lite.
option(RESTINIO_USE_SYSTEM_EXPECTED_LITE "Assume expected lite is available as system headers" OFF)

message(STATUS "RESTINIO_USE_SYSTEM_STANDALONE_ASIO: ${RESTINIO_USE_SYSTEM_STANDALONE_ASIO}")
message(STATUS "RESTINIO_USE_SYSTEM_BOOST_ASIO:      ${RESTINIO_USE_SYSTEM_BOOST_ASIO}")
message(STATUS "RESTINIO_USE_SYSTEM_LLHTTP:          ${RESTINIO_USE_SYSTEM_LLHTTP}")
message(STATUS "RESTINIO_USE_SYSTEM_FMT:             ${RESTINIO_USE_SYSTEM_FMT}")
message(STATUS "RESTINIO_USE_SYSTEM_EXPECTED_LITE:   ${RESTINIO_USE_SYSTEM_EXPECTED_LITE}")

set(RESTINIO_ASIO_SOURCE "standalone" CACHE STRING "What source of ASIO to use (standalone, boost)")
set(RESTINIO_ASIO_SOURCE_VALUES "standalone;boost")
set_property(CACHE RESTINIO_ASIO_SOURCE PROPERTY STRINGS ${RESTINIO_ASIO_SOURCE_VALUES})

set(RESTINIO_FMT_TARGET "" CACHE STRING "What target of FMT to use (fmt::fmt-header-only, fmt::fmt)")
set(RESTINIO_FMT_TARGET_VALUES ";fmt::fmt-header-only;fmt::fmt")
set_property(CACHE RESTINIO_FMT_TARGET PROPERTY STRINGS ${RESTINIO_FMT_TARGET_VALUES})

message(STATUS "RESTINIO_ASIO_SOURCE: ${RESTINIO_ASIO_SOURCE}")

if (RESTINIO_FMT_TARGET STREQUAL "" )
    message(STATUS "RESTINIO_FMT_TARGET:  will be decided dynamically")
else ()
    message(STATUS "RESTINIO_FMT_TARGET:  ${RESTINIO_FMT_TARGET}")
endif ()


list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

if (RESTINIO_ASIO_SOURCE STREQUAL "standalone")
    if (NOT RESTINIO_USE_SYSTEM_STANDALONE_ASIO)
        message(STATUS "Finding ASIO package...")
        set(RESTINIO_ASIO_PATH_HINT ${CMAKE_CURRENT_SOURCE_DIR}/asio)
        find_package(asio REQUIRED)
        if (NOT asio_FOUND)
            message(FATAL_ERROR "Unable to find ASIO")
        endif()
    endif ()
else ()
    # TODO
endif ()

if (NOT RESTINIO_USE_SYSTEM_LLHTTP)
    message(STATUS "Add subdirectory nodejs/llhttp")
    add_subdirectory(nodejs/llhttp)
endif ()

if (NOT RESTINIO_USE_SYSTEM_FMT)
    message(STATUS "Add subdirectory fmt")
    add_subdirectory(fmt)
endif ()

if (NOT RESTINIO_USE_SYSTEM_EXPECTED_LITE)
    message(STATUS "Finding expected-lite package...")
    set(RESTINIO_EXPECTED_LITE_PATH_HINT ${CMAKE_CURRENT_SOURCE_DIR}/expected-lite)
    find_package(expected-lite REQUIRED)
    if (NOT expected-lite_FOUND)
        message(FATAL_ERROR "Unable to find expected-lite")
    endif()
endif ()

if (RESTINIO_WITH_SOBJECTIZER)
    SET(SOBJECTIZER_BUILD_STATIC ON)
    add_subdirectory(so_5)
endif ()

find_package(ZLIB)
if (ZLIB_FOUND)
  message(STATUS "ZLIB include dir: ${ZLIB_INCLUDE_DIRS}")
  message(STATUS "ZLIB libraries: ${ZLIB_LIBRARIES}")
endif ()

find_package(OpenSSL)
IF ( OPENSSL_FOUND )
  message(STATUS "OpenSSL include dir: ${OPENSSL_INCLUDE_DIR}")
  message(STATUS "OpenSSL libraries: ${OPENSSL_LIBRARIES}")
ENDIF ( OPENSSL_FOUND )

# PCRE
find_package(PCRE)
IF (PCRE_FOUND)
  message( STATUS "PCRE_LIBRARIES='" ${PCRE_LIBRARIES} "'" )
  message( STATUS "PCRE_INCLUDE_DIRS='" ${PCRE_INCLUDE_DIRS} "'" )
ENDIF ()

# PCRE2
find_package(PCRE2)
IF (PCRE2_FOUND)
  message( STATUS "PCRE2_LIBRARIES='" ${PCRE2_LIBRARIES} "'" )
  message( STATUS "PCRE2_INCLUDE_DIRS='" ${PCRE2_INCLUDE_DIRS} "'" )
ENDIF ()

# PCRE2
find_package(PCRE2)
IF (PCRE2_FOUND)
  message( STATUS "PCRE2_LIBRARIES='" ${PCRE2_LIBRARIES} "'" )
  message( STATUS "PCRE2_INCLUDE_DIRS='" ${PCRE2_INCLUDE_DIRS} "'" )
ENDIF ()

message("========================================")
message(STATUS "Defining restinio target...")
add_subdirectory(restinio)

if (RESTINIO_TEST)
    enable_testing()
    add_subdirectory(test)
endif ()

if (RESTINIO_SAMPLE)
    include_directories("${CMAKE_SOURCE_DIR}/clara")
    add_subdirectory(sample)
endif ()

if (RESTINIO_BENCHMARK)
    add_subdirectory(benches)
endif ()
