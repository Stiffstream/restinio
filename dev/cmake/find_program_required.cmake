# Runs `find_program` and checks that the program was found.
#
# find_program_required(
#     NAME clang-tidy                       # A generic name of the application.
#     SEARCH_NAME  ${CLANG_TIDY_EXACT_NAME} # A name used for search.
#     PATH clang_tidy_exe                   # A name of the resulting executable.
# )
function(find_program_required)

    set(one_value_args NAME SEARCH_NAME PATH)
    cmake_parse_arguments(fpr_param "${options}" "${one_value_args}" "" ${ARGN})

    message(STATUS "SEARCH ${fpr_param_NAME} => ${fpr_param_SEARCH_NAME}")
    find_program(${fpr_param_PATH} NAMES ${fpr_param_SEARCH_NAME})

    if (NOT ${fpr_param_PATH} )
        message(STATUS "${fpr_param_NAME} not found using SEARCH_NAME: '${fpr_param_SEARCH_NAME}' "
                       "will search for ${fpr_param_NAME} specifically")
        find_program(${fpr_param_PATH} NAMES ${fpr_param_NAME})
    endif()

    if (${fpr_param_PATH})
        message(STATUS "${fpr_param_NAME} found: ${${fpr_param_PATH}}")
    else ()
        # A required application was not found!
        message(FATAL_ERROR "Failed to find ${fpr_param_NAME}")
    endif ()
endfunction()
