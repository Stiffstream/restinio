cmake_minimum_required(VERSION 3.8)

if(NOT RESTINIO_LIBRARY_NAME)
    # That must be the case we RESTinio is used as a dependency.
    set(RESTINIO_LIBRARY_NAME restinio)
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/version.cmake)

project(${RESTINIO_LIBRARY_NAME}
        VERSION ${RESTINIO_LIB_VERSION}
        LANGUAGES CXX)

# ====================================================================
# Version
# ====================================================================

set(RESTINIO_LIB_CONFIG "${RESTINIO_LIBRARY_NAME}/version.hpp")

set(version_header_include_path "${PROJECT_BINARY_DIR}/include")
set(version_header_path "${version_header_include_path}/${RESTINIO_LIB_CONFIG}")
configure_file(
    "${CMAKE_CURRENT_LIST_DIR}/version.hpp.in"
    ${version_header_path})

# ====================================================================
# RESTinio target and it's attributes
# ====================================================================

add_library(${RESTINIO_LIBRARY_NAME} INTERFACE)
add_library(${RESTINIO_LIBRARY_NAME}::${RESTINIO_LIBRARY_NAME} ALIAS ${RESTINIO_LIBRARY_NAME})

target_include_directories(
    ${RESTINIO_LIBRARY_NAME} INTERFACE
    # TODO: that might be a good idea to move
    #       headers to include dir so the next dir is not pointing to
    #       the parent dir.
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/..>
    $<BUILD_INTERFACE:${version_header_include_path}>
    $<INSTALL_INTERFACE:include>)

if (RESTINIO_EXPLICIT_CPPSTD)
    if ("${RESTINIO_EXPLICIT_CPPSTD}" STREQUAL "17" )
        target_compile_features(${RESTINIO_LIBRARY_NAME} INTERFACE cxx_std_17)
    elseif ("${RESTINIO_EXPLICIT_CPPSTD}" STREQUAL "20")
        target_compile_features(${RESTINIO_LIBRARY_NAME} INTERFACE cxx_std_20)
    elseif ("${RESTINIO_EXPLICIT_CPPSTD}" STREQUAL "23")
        target_compile_features(${RESTINIO_LIBRARY_NAME} INTERFACE cxx_std_23)
    else ()
        message(FATAL_ERROR
                "invalid RESTINIO_EXPLICIT_CPPSTD (value ='${RESTINIO_EXPLICIT_CPPSTD}')"
                "must be on of: [17, 20, 23]")
    endif ()
endif()

# --------------------------------------------------------------------
# Handle ASIO dependency.
if (RESTINIO_ASIO_SOURCE STREQUAL "standalone")
    if(RESTINIO_USE_SYSTEM_STANDALONE_ASIO)
        message(STATUS "${RESTINIO_LIBRARY_NAME} target assumes standalone asio is available in system headers")
    elseif (TARGET asio::asio)
        target_link_libraries(${RESTINIO_LIBRARY_NAME} INTERFACE asio::asio)
        message(STATUS "${RESTINIO_LIBRARY_NAME} target will depend on 'asio::asio' target")
    elseif (asio_INCLUDE_DIRS)
        target_include_directories(${RESTINIO_LIBRARY_NAME} INTERFACE ${asio_INCLUDE_DIRS})
        message(STATUS "${RESTINIO_LIBRARY_NAME} target will use asio_INCLUDE_DIRS include dirs: ${asio_INCLUDE_DIRS}")
    else ()
        message(FATAL_ERROR
                "Unable to identify asio::asio or asio_INCLUDE_DIRS "
                "(RESTINIO_ASIO_SOURCE=standalone), giving up")
    endif()
elseif (RESTINIO_ASIO_SOURCE STREQUAL "boost")
    if(RESTINIO_USE_SYSTEM_BOOST_ASIO)
        message(STATUS "${RESTINIO_LIBRARY_NAME} target assumes Boost::asio is available in system headers")
    elseif (TARGET Boost::headers)
        target_link_libraries(${RESTINIO_LIBRARY_NAME} INTERFACE Boost::headers)
        message(STATUS "${RESTINIO_LIBRARY_NAME} target will depend on 'Boost::headers' target")
    elseif (TARGET Boost::asio)
        target_link_libraries(${RESTINIO_LIBRARY_NAME} INTERFACE Boost::asio)
        message(STATUS "${RESTINIO_LIBRARY_NAME} target will depend on 'Boost::asio' target")
    elseif (Boost_INCLUDE_DIRS)
        target_include_directories(${RESTINIO_LIBRARY_NAME} INTERFACE ${Boost_INCLUDE_DIRS})
        message(STATUS "${RESTINIO_LIBRARY_NAME} target will use Boost_INCLUDE_DIRS include dirs: ${Boost_INCLUDE_DIRS}")
    else ()
        message(FATAL_ERROR
                "Unable to identify Boost::headers, Boost::asio or Boost_INCLUDE_DIRS "
                "(RESTINIO_ASIO_SOURCE=boost), giving up")
    endif ()
else ()
    message(FATAL_ERROR "Invalid value for RESTINIO_ASIO_SOURCE (${RESTINIO_ASIO_SOURCE})")
endif ()
# --------------------------------------------------------------------

# --------------------------------------------------------------------
# Handle FMT dependency.
if(RESTINIO_USE_SYSTEM_LLHTTP)
    message(STATUS "Using system llhttp")
    # target_compile_definitions(${RESTINIO_LIBRARY_NAME} INTERFACE FMT_HEADER_ONLY=0 )
    target_link_libraries(${RESTINIO_LIBRARY_NAME} INTERFACE llhttp)
elseif (TARGET llhttp::llhttp)
    target_link_libraries(${RESTINIO_LIBRARY_NAME} INTERFACE llhttp::llhttp)
    message(STATUS "${RESTINIO_LIBRARY_NAME} target will depend on 'llhttp::llhttp' target")
elseif (TARGET unofficial::llhttp::llhttp)
    target_link_libraries(${RESTINIO_LIBRARY_NAME} INTERFACE unofficial::llhttp::llhttp)
    message(STATUS "${RESTINIO_LIBRARY_NAME} target will depend on 'unofficial::llhttp::llhttp' target")
else ()
    message(FATAL_ERROR
            "Unable to identify how to depend on llhttp. "
            "Giving up...")
endif()
# --------------------------------------------------------------------

# --------------------------------------------------------------------
# Handle FMT dependency.
if(RESTINIO_USE_SYSTEM_FMT)
    if (RESTINIO_FMT_LIB_LINK_NAME)
        message(STATUS "Using system fmt with linking to ${RESTINIO_FMT_LIB_LINK_NAME}")
        # target_compile_definitions(${RESTINIO_LIBRARY_NAME} INTERFACE FMT_HEADER_ONLY=0 )
        target_link_libraries(${RESTINIO_LIBRARY_NAME} INTERFACE ${RESTINIO_FMT_LIB_LINK_NAME})
    else()
        message(STATUS "Using system fmt (assume header-only)")
        target_compile_definitions(${RESTINIO_LIBRARY_NAME} INTERFACE FMT_HEADER_ONLY=1 )
    endif()
elseif (NOT "${RESTINIO_FMT_TARGET}" STREQUAL "" )
    target_link_libraries(${RESTINIO_LIBRARY_NAME} INTERFACE ${RESTINIO_FMT_TARGET})
    message(STATUS "${RESTINIO_LIBRARY_NAME} target will depend on '${RESTINIO_FMT_TARGET}' target")
elseif (TARGET fmt::fmt)
    target_link_libraries(${RESTINIO_LIBRARY_NAME} INTERFACE fmt::fmt)
    message(STATUS "${RESTINIO_LIBRARY_NAME} target will depend on 'fmt::fmt' target")
else ()
    message(FATAL_ERROR
            "Unable to identify how to depend on FMT. "
            "Giving up...")
endif()
# --------------------------------------------------------------------

# --------------------------------------------------------------------
# Handle expected lite dependency.
if ("${RESTINIO_EXPLICIT_CPPSTD}" STREQUAL "17"
    OR "${RESTINIO_EXPLICIT_CPPSTD}" STREQUAL "20"
    OR "CMAKE_CXX_STANDARD" STREQUAL "17"
    OR "CMAKE_CXX_STANDARD" STREQUAL "20")

    # TODO: Consider expected that comes with C++23.
    #       In that case the next fragment must be places under a given
    #       conditional block.
endif()

if(RESTINIO_USE_SYSTEM_EXPECTED_LITE)
    message(STATUS "Using system expected-lite")
elseif (TARGET expected-lite::expected-lite)
    target_link_libraries(${RESTINIO_LIBRARY_NAME} INTERFACE expected-lite::expected-lite)
    message(STATUS "${RESTINIO_LIBRARY_NAME} target will depend on 'nonstd::expected-lite' target")
elseif (TARGET nonstd::expected-lite)
    target_link_libraries(${RESTINIO_LIBRARY_NAME} INTERFACE nonstd::expected-lite)
    message(STATUS "${RESTINIO_LIBRARY_NAME} target will depend on 'nonstd::expected-lite' target")
elseif (nonstd_expected-lite_INCLUDE_DIRS)
    target_include_directories(${RESTINIO_LIBRARY_NAME} INTERFACE ${nonstd_expected-lite_INCLUDE_DIRS})
    message(STATUS "${RESTINIO_LIBRARY_NAME} target will use nonstd_expected-lite_INCLUDE_DIRS include dirs: ${nonstd_expected-lite_INCLUDE_DIRS}")
elseif (nonstd_INCLUDE_DIRS)
    target_include_directories(${RESTINIO_LIBRARY_NAME} INTERFACE ${nonstd_INCLUDE_DIRS})
    message(STATUS "${RESTINIO_LIBRARY_NAME} target will use nonstd_INCLUDE_DIRS include dirs: ${nonstd_INCLUDE_DIRS}")
else ()
    message(FATAL_ERROR
            "Unable to identify nonstd::expected-lite, expected-lite::expected-lite, nonstd_INCLUDE_DIRS or nonstd_expected-lite_INCLUDE_DIRS. "
            "Giving up...")
endif()
# --------------------------------------------------------------------


if (RESTINIO_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

  set(RESTINIO_CMAKE_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/restinio CACHE STRING
      "Installation directory for cmake files, relative to ${CMAKE_INSTALL_PREFIX}.")
  set(RESTINIO_VERSION_CONFIG ${PROJECT_BINARY_DIR}/restinio-config-version.cmake)
  set(RESTINIO_PROJECT_CONFIG ${PROJECT_BINARY_DIR}/restinio-config.cmake)

  SET(RESTINIO_INC_DIR ${CMAKE_INSTALL_INCLUDEDIR}/restinio/ CACHE STRING
      "Installation directory for include files, relative to ${CMAKE_INSTALL_PREFIX}.")

  # Generate the version, config and target files into the build directory.
  write_basic_package_version_file(
      ${RESTINIO_VERSION_CONFIG}
      VERSION ${RESTINIO_LIB_VERSION}
      COMPATIBILITY ExactVersion )

  configure_package_config_file(
      "cmake/restinio-config.cmake.in"
      ${RESTINIO_PROJECT_CONFIG}
      INSTALL_DESTINATION ${RESTINIO_CMAKE_DIR}
      PATH_VARS RESTINIO_LIB_VERSION)

    # Install version, config and target files.
    install(
        FILES ${RESTINIO_PROJECT_CONFIG} ${RESTINIO_VERSION_CONFIG}
        DESTINATION ${RESTINIO_CMAKE_DIR})

    SET(RESTINIO_INSTALL_TARGETS ${RESTINIO_LIBRARY_NAME})

    install(
        TARGETS ${RESTINIO_INSTALL_TARGETS}
        EXPORT RESTINIO_ALL_TARGETS
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
    )

    install(
        EXPORT RESTINIO_ALL_TARGETS
        NAMESPACE restinio::
        FILE restinio-targets.cmake
        DESTINATION ${RESTINIO_CMAKE_DIR} )

    file(GLOB_RECURSE RESTINIO_HEADERS_ALL RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.?pp)
    foreach (HEADER_FILE ${RESTINIO_HEADERS_ALL} )
      get_filename_component( DIR ${HEADER_FILE} PATH )
      install( FILES ${HEADER_FILE} DESTINATION ${RESTINIO_INC_DIR}/${DIR} )
    endforeach ()
endif ()
