# - Try to find http-parser
#
# Defines the following variables:
#
# HTTP_PARSER_FOUND - system has http-parser
# HTTP_PARSER_INCLUDE_DIR - the http-parser include directory
# HTTP_PARSER_LIBRARIES - Link these to use http-parser

# Find the header and library
FIND_PATH(HTTP_PARSER_INCLUDE_DIR NAMES http_parser.h)
FIND_LIBRARY(HTTP_PARSER_LIBRARY NAMES http_parser libhttp_parser)

# Handle the QUIETLY and REQUIRED arguments and set HTTP_PARSER_FOUND
# to TRUE if all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(HTTP_Parser REQUIRED_VARS HTTP_PARSER_INCLUDE_DIR HTTP_PARSER_LIBRARY)

# Hide advanced variables
MARK_AS_ADVANCED(HTTP_PARSER_INCLUDE_DIR HTTP_PARSER_LIBRARY)

# Set standard variables
IF (HTTP_PARSER_FOUND)
	SET(HTTP_PARSER_LIBRARIES ${HTTP_PARSER_LIBRARY})
	SET(HTTP_PARSER_INCLUDE_DIRS ${HTTP_PARSER_INCLUDE_DIR})
	MESSAGE(STATUS "http_parser libraries: ${HTTP_PARSER_LIBRARIES}")
	MESSAGE(STATUS "http_parser include dir: ${HTTP_PARSER_INCLUDE_DIRS}")
ENDIF()

