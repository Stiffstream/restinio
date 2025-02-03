cmake_minimum_required(VERSION 3.10)

if(NOT RESTINIO_LIBRARY_NAME)
    # That must be the case when RESTinio is used as a dependency.
    set(RESTINIO_LIBRARY_NAME restinio)
endif()

# ====================================================================
# Version
include(${CMAKE_CURRENT_SOURCE_DIR}/version.cmake)
message(STATUS "RESTINIO_VERSION: ${RESTINIO_VERSION}")
# ====================================================================

project(${RESTINIO_LIBRARY_NAME}
        VERSION ${RESTINIO_VERSION}
        LANGUAGES CXX)

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
    if (RESTINIO_DEP_STANDALONE_ASIO STREQUAL "system")
        message(STATUS "${RESTINIO_LIBRARY_NAME} target assumes standalone asio is available in system headers")
    elseif (TARGET asio::asio)
        target_link_libraries(${RESTINIO_LIBRARY_NAME} INTERFACE asio::asio)
        message(STATUS "${RESTINIO_LIBRARY_NAME} target will depend on 'asio::asio' target")
    elseif (asio_INCLUDE_DIRS)
        target_include_directories(${RESTINIO_LIBRARY_NAME} INTERFACE ${asio_INCLUDE_DIRS})
        message(STATUS "${RESTINIO_LIBRARY_NAME} target will use asio_INCLUDE_DIRS include dirs: ${asio_INCLUDE_DIRS}")
    else ()
        message(FATAL_ERROR
                "Unable to identify how to depend on standalone ASIO "
                "Giving up...")
    endif()
elseif (RESTINIO_ASIO_SOURCE STREQUAL "boost")
    if (RESTINIO_DEP_BOOST_ASIO STREQUAL "system")
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
                "Unable to identify how to depend on Boost::asio. "
                "Giving up...")
    endif ()

    target_compile_definitions(${RESTINIO_LIBRARY_NAME} INTERFACE RESTINIO_USE_BOOST_ASIO)
else ()
    message(FATAL_ERROR "Invalid value for RESTINIO_ASIO_SOURCE (${RESTINIO_ASIO_SOURCE})")
endif ()
# --------------------------------------------------------------------

# --------------------------------------------------------------------
# Handle LLHTTP dependency.
if (RESTINIO_DEP_LLHTTP STREQUAL "system")
    message(STATUS "Using system llhttp")
    if (RESTINIO_LLHTTP_LIB_LINK_NAME)
        message(STATUS "Using system llhttp with linking to custom lib name: ${RESTINIO_LLHTTP_LIB_LINK_NAME}")
        target_link_libraries(${RESTINIO_LIBRARY_NAME} INTERFACE ${RESTINIO_LLHTTP_LIB_LINK_NAME})
    else()
        target_link_libraries(${RESTINIO_LIBRARY_NAME} INTERFACE llhttp)
    endif()
elseif (TARGET llhttp::llhttp)
    target_link_libraries(${RESTINIO_LIBRARY_NAME} INTERFACE llhttp::llhttp)
    message(STATUS "${RESTINIO_LIBRARY_NAME} target will depend on 'llhttp::llhttp' target")
elseif (TARGET llhttp::llhttp_static)
    target_link_libraries(${RESTINIO_LIBRARY_NAME} INTERFACE llhttp::llhttp_static)
    message(STATUS "${RESTINIO_LIBRARY_NAME} target will depend on 'llhttp::llhttp_static' target")
elseif (TARGET llhttp::llhttp_dynamic)
    target_link_libraries(${RESTINIO_LIBRARY_NAME} INTERFACE llhttp::llhttp_dynamic)
    message(STATUS "${RESTINIO_LIBRARY_NAME} target will depend on 'llhttp::llhttp_dynamic' target")
else ()
    message(FATAL_ERROR
            "Unable to identify how to depend on llhttp. "
            "Giving up...")
endif()
# --------------------------------------------------------------------

# --------------------------------------------------------------------
# Handle FMT dependency.
if (RESTINIO_DEP_FMT STREQUAL "system")
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

if (RESTINIO_DEP_EXPECTED_LITE STREQUAL "system")
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
            "Unable to identify how to depend on nonstd::expected-lite. "
            "Giving up...")
endif()
# --------------------------------------------------------------------


if (RESTINIO_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

  set(RESTINIO_CMAKE_INSTALL_DEST_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/restinio CACHE STRING
      "Installation directory for cmake files, relative to ${CMAKE_INSTALL_PREFIX}.")

  set(restinio_version_config_tmp_file ${PROJECT_BINARY_DIR}/restinio-config-version.cmake)
  set(restinio_project_config_tmp_file ${PROJECT_BINARY_DIR}/restinio-config.cmake)

  SET(restinio_install_include_dir ${CMAKE_INSTALL_INCLUDEDIR}/restinio/ CACHE STRING
      "Installation directory for include files, relative to ${CMAKE_INSTALL_PREFIX}.")

  # Generate the version, config and target files into the build directory.
  write_basic_package_version_file(
      ${restinio_version_config_tmp_file}
      VERSION ${RESTINIO_VERSION}
      COMPATIBILITY SameMinorVersion )

  configure_package_config_file(
      "cmake/restinio-config.cmake.in"
      ${restinio_project_config_tmp_file}
      INSTALL_DESTINATION ${RESTINIO_CMAKE_INSTALL_DEST_DIR}
      PATH_VARS RESTINIO_VERSION)

    # Install version, config and target files.
    install(
        FILES ${restinio_project_config_tmp_file} ${restinio_version_config_tmp_file}
        DESTINATION ${RESTINIO_CMAKE_INSTALL_DEST_DIR})

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
        DESTINATION ${RESTINIO_CMAKE_INSTALL_DEST_DIR} )

    file(GLOB_RECURSE RESTINIO_HEADERS_ALL RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.?pp)
    foreach (HEADER_FILE ${RESTINIO_HEADERS_ALL} )
      get_filename_component( DIR ${HEADER_FILE} PATH )
      install( FILES ${HEADER_FILE} DESTINATION ${restinio_install_include_dir}/${DIR} )
    endforeach ()
endif ()
