# Inspiration
# https://github.com/bilke/cmake-modules/blob/master/CodeCoverage.cmake
# https://gcovr.com/en/stable/guide.html#options
#
# Adds code coverage analysis to all the project starting from a
# point make_code_coverage_targets() function is called.
# By convention it is assumed that this the directory is a root of the project.
#
# add_code_coverage_targets(
#     NAME
#         prjcoverage                         # Base name for coverage targets:
#                                             # prjcoverage_xml
#                                             # prjcoverage_html
#     FILTERS
#        '${CMAKE_SOURCE_DIR}/include/.*'     # Path filters for files
#        '${CMAKE_SOURCE_DIR}/src/.*'         # considered for coverage
#     XML_REPORT_FILE
#         "{CMAKE_SOURCE_DIR}/coverage.xml"   # Explicit path to file where
#                                             # where to create a XML coverage
#                                             # report. If not defined then
#                                             # report is created in the root
#                                             # of the project with filename
#                                             # "<NAME>.xml".
#     HTML_REPORT_DIR
#         "{CMAKE_SOURCE_DIR}/coverage"               # Explicit path to directory where
#                                                     # where to create an HTML coverage
#                                                     # report. If not defined then
#                                                     # report is created in the "<NAME>"
#                                                     # directory in the root of the project.
#     HTML_TITLE
#         "My Project report"                         # HTML report page title.
# )
function (make_code_coverage_targets)
    #coverage doesn't support ninja build
    if (CMAKE_GENERATOR STREQUAL "Ninja")
        message(FATAL_ERROR "Can not use coverage with ninja")
    endif()

# --------------------------------------------------------------------
    set(one_value_args NAME XML_REPORT_FILE HTML_REPORT_DIR HTML_TITLE)
    set(multi_value_args FILTERS)

    cmake_parse_arguments(Coverage "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})

    if( NOT Coverage_NAME)
        message(FATAL_ERROR "make_code_coverage_targets failed: "
                            "NAME parameter must be defined")
    endif ()

    if (NOT Coverage_XML_REPORT_FILE)
        set( Coverage_XML_REPORT_FILE "${CMAKE_SOURCE_DIR}/${Coverage_NAME}.xml")
    endif ()
    message(STATUS "XML coverage report file: ${Coverage_XML_REPORT_FILE}")

    if (NOT Coverage_HTML_REPORT_DIR)
        set( Coverage_HTML_REPORT_DIR "${CMAKE_SOURCE_DIR}/${Coverage_NAME}")
    endif ()
    message(STATUS "HTML coverage report directory: ${Coverage_HTML_REPORT_DIR}")
# --------------------------------------------------------------------

    find_program_required(
        NAME gcov
        SEARCH_NAME ${TOOLING_CODE_COVERAGE_GCOV}
        PATH gcov_exe)

    find_program_required(
        NAME gcovr
        SEARCH_NAME ${TOOLING_CODE_COVERAGE_GCOVR}
        PATH gcovr_exe)

# --------------------------------------------------------------------
    # Instrument our build:
    list(APPEND extra_compile_options -fprofile-arcs -ftest-coverage)

    # Add compile and link flags as global.

    add_compile_options(${extra_compile_options})
    message(STATUS "Coverage instrumentation: add global compile options: ${extra_compile_options}")

    list(APPEND extra_ling_options --coverage -lgcov)
    add_link_options(${extra_ling_options})
    message(STATUS "Coverage instrumentation: add global link options: ${extra_ling_options}")
# --------------------------------------------------------------------

    # Setup target
    add_custom_target(${Coverage_NAME}_ctest_run
        # Run tests
        COMMAND ctest T test
        DEPENDS ${Coverage_DEPENDENCIES}
        COMMENT "Running test to gather coverage data"
    )

    foreach (filt ${Coverage_FILTERS} )
        list(APPEND coverage_filters "-f" ${filt})
    endforeach ()

    message(STATUS "Coverage filters: ${coverage_filters}")

    list(APPEND base_xml_report_cmd
        ${gcovr_exe}
        --root ${CMAKE_SOURCE_DIR}
        ${coverage_filters}
        --gcov-executable ${gcov_exe}
        --xml-pretty
        --print-summary
        --exclude-unreachable-branches
        -o ${Coverage_XML_REPORT_FILE})

    add_custom_target(${Coverage_NAME}_xml
        COMMAND ${base_xml_report_cmd}
                --delete # The gcda files are generated when
                         # the instrumented program is executed.
                         # This flag removes coverage data
                         # so that the next build of the target
                         # collects the date from scratch.
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
        COMMENT "Processing code coverage counters and generating XML report.\n"
    )
    add_dependencies(${Coverage_NAME}_xml ${Coverage_NAME}_ctest_run)

    if (Coverage_HTML_TITLE)
        list(APPEND title_args "--html-title" "${Coverage_HTML_TITLE}")
    endif()


    list(APPEND base_html_report_cmd
        ${gcovr_exe}
        --root ${CMAKE_SOURCE_DIR}
        ${coverage_filters}
        ${title_args}
        --gcov-executable ${gcov_exe}
        --html --html-details
        --exclude-unreachable-branches
        -o ${Coverage_HTML_REPORT_DIR}/index.html)

    add_custom_target(${Coverage_NAME}_html
        COMMAND ${CMAKE_COMMAND} -E make_directory ${Coverage_HTML_REPORT_DIR}
        COMMAND ${base_html_report_cmd}
                --delete # The gcda files are generated when
                         # the instrumented program is executed.
                         # This flag removes coverage data
                         # so that the next build of the target
                         # collects the date from scratch.
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
        DEPENDS ${Coverage_DEPENDENCIES}
        COMMENT "Processing code coverage counters and generating HTML report.\n"
    )
    add_dependencies(${Coverage_NAME}_html ${Coverage_NAME}_ctest_run)

    add_custom_target(${Coverage_NAME}_all
        COMMAND ${base_xml_report_cmd}

        COMMAND ${CMAKE_COMMAND} -E make_directory ${Coverage_HTML_REPORT_DIR}
        COMMAND ${base_html_report_cmd} --delete

        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
        DEPENDS ${Coverage_DEPENDENCIES}
        COMMENT "Processing code coverage counters and generating reports (HTML, XML).\n"
    )
    add_dependencies(${Coverage_NAME}_all ${Coverage_NAME}_ctest_run)

    # Show info where to find the report
    add_custom_command(TARGET ${Coverage_NAME}_html POST_BUILD
        COMMAND ;
        COMMENT "Open ${Coverage_HTML_REPORT_DIR}/index.html in your browser to view the coverage report."
    )

    # Show info where to find the report
    add_custom_command(TARGET ${Coverage_NAME}_all POST_BUILD
        COMMAND ;
        COMMENT "Open ${Coverage_HTML_REPORT_DIR}/index.html in your browser to view the coverage report."
    )

    message(STATUS "CODE COVERAGE TARGETS CREATED...\n"
                   "   To build XML code coverage report run:\n"
                   "       cmake --build <build_dir> --target ${Coverage_NAME}_xml\n"
                   "   To build HTML code coverage report run:\n"
                   "       cmake --build <build_dir> --target ${Coverage_NAME}_html\n"
                   "   To build all code coverage reports in one go run:\n"
                   "       cmake --build <build_dir> --target ${Coverage_NAME}_all")
endfunction ()
