macro(handle_explicit_libcxx_if_necessary PARAM_EXPLICIT_LIBCXX)

    if (PARAM_EXPLICIT_LIBCXX STREQUAL "libstdc++"
        OR PARAM_EXPLICIT_LIBCXX STREQUAL "libstdc++11"
        OR PARAM_EXPLICIT_LIBCXX STREQUAL "libc++")

        message(STATUS "Using libcxx: ${PARAM_EXPLICIT_LIBCXX}")
        message(STATUS "Using libcxx: CMAKE_CXX_COMPILER_ID=${CMAKE_CXX_COMPILER_ID}")
    else()
        message(FATAL_ERROR "invalid value of PARAM_EXPLICIT_LIBCXX ('${PARAM_EXPLICIT_LIBCXX}')"
                            "must be one of: [libstdc++, libstdc++11, libc++]" )
    endif()

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        if(PARAM_EXPLICIT_LIBCXX STREQUAL "libstdc++" OR PARAM_EXPLICIT_LIBCXX STREQUAL "libstdc++11" )
            message(STATUS "libcxx adjustment: CMAKE_CXX_FLAGS += '-stdlib=libstdc++'")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libstdc++")
        elseif(PARAM_EXPLICIT_LIBCXX STREQUAL "libc++")
            message(STATUS "libcxx adjustment: CMAKE_CXX_FLAGS += '-stdlib=libc++'")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
            message(STATUS "libcxx adjustment: CMAKE_EXE_LINKER_FLAGS += '-stdlib=libc++'")
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++")
        endif()
    endif()

    if(PARAM_EXPLICIT_LIBCXX STREQUAL "libstdc++11")
        message(STATUS "libcxx adjustment: -D_GLIBCXX_USE_CXX11_ABI=1")
        add_definitions(-D_GLIBCXX_USE_CXX11_ABI=1)
    elseif(PARAM_EXPLICIT_LIBCXX STREQUAL "libstdc++")
        message(STATUS "libcxx adjustment: -D_GLIBCXX_USE_CXX11_ABI=0")
        add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
    endif()
endmacro()
