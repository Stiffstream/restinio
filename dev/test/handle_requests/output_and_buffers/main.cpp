/*
	restinio
*/

/*!
	Test method detection.
*/

#define CATCH_CONFIG_MAIN
#include <catch/catch.hpp>

#include <asio.hpp>

#include <restinio/all.hpp>

#include <test/common/utest_logger.hpp>
#include <test/common/pub.hpp>


TEST_CASE( "FAKE" , "[fake]" )
{
	REQUIRE( false );
}
