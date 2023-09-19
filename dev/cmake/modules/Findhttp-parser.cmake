# - Try to find http-parser
#
# Defines the following variables:
#
# LLHTTP_FOUND - system has http-parser
# LLHTTP_INCLUDE_DIR - the http-parser include directory
# LLHTTP_LIBRARIES - Link these to use http-parser

# Find the header and library
FIND_PATH(LLHTTP_INCLUDE_DIR NAMES llhttp.h)
FIND_LIBRARY(LLHTTP_LIBRARY NAMES llhttp libllhttp)

# Handle the QUIETLY and REQUIRED arguments and set LLHTTP_FOUND
# to TRUE if all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LLHTTP REQUIRED_VARS LLHTTP_INCLUDE_DIR LLHTTP_LIBRARY)

# Hide advanced variables
MARK_AS_ADVANCED(LLHTTP_INCLUDE_DIR LLHTTP_LIBRARY)

# Set standard variables
IF (LLHTTP_FOUND)
	SET(LLHTTP_LIBRARIES ${LLHTTP_LIBRARY})
	SET(LLHTTP_INCLUDE_DIRS ${LLHTTP_INCLUDE_DIR})
	MESSAGE(STATUS "llhttp libraries: ${LLHTTP_LIBRARIES}")
	MESSAGE(STATUS "llhttp include dir: ${LLHTTP_INCLUDE_DIRS}")
ENDIF()

