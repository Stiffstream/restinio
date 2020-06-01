include(CheckCXXSourceCompiles)

check_cxx_source_compiles("
    #include <atomic>
    int main(int argc, char*[] ) {
        std::atomic<int> x;
        x = argc;
        x--;
        return x;
    }"
    have_atomic_by_default)

if(have_atomic_by_default)
    message(STATUS "atomic routine linked by default")
else ()
    message(STATUS "Needs explicit atomic linkage")
    find_library(ATOMIC NAMES atomic libatomic.so libatomic.so.1)
    if(ATOMIC)
        set(LIBATOMIC ${ATOMIC})
        message(STATUS "Found libatomic: TRUE")
    else ()
        message(WARNING "Found libatomic: FALSE")
    endif()
endif()

function(link_atomic_if_necessary targetName)
    if (NOT have_atomic_by_default)
        target_link_libraries(${targetName} PRIVATE ${LIBATOMIC})
    endif ()
endfunction(link_atomic_if_necessary)
