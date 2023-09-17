#pragma once

#define RESTINIO_VERSION_CODE( major, minor, patch ) \
    ( ( ( major ) * 1000000ull ) + ( ( minor ) *1000ull ) + ( patch ) )

#define RESTINIO_VERSION_MAJOR 0ull
#define RESTINIO_VERSION_MINOR 7ull
#define RESTINIO_VERSION_PATCH 0ull

#define RESTINIO_VERSION \
    RESTINIO_VERSION_CODE( RESTINIO_VERSION_MAJOR, RESTINIO_VERSION_MINOR, RESTINIO_VERSION_PATCH )
