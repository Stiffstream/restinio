/*
	restinio
*/

/*!
	Echo server.
*/

#include <restinio/string_view.inl>

#define CATCH_CONFIG_MAIN
#include <catch/catch.hpp>

using restinio::string_view_t;

TEST_CASE( "Test constructors" , "[string_view][constructor]" )
{
	SECTION( "string_view_t()" )
	{
		string_view_t view;

		REQUIRE( view.empty() );
		REQUIRE( view.data() == nullptr );
		REQUIRE( view.size() == 0 );
	}

	SECTION( "string_view_t( const char *, size_type count )" )
	{
		const char * text = "Test string";
		string_view_t::size_type count = 4;
		string_view_t view{ text, count };

		REQUIRE_FALSE( view.empty() );
		REQUIRE( view.data() == text );
		REQUIRE( view.size() == 4 );
	}

	SECTION( "string_view_t( const char *)" )
	{
		const char * text = "Test";
		string_view_t view{ text };

		REQUIRE_FALSE( view.empty() );
		REQUIRE( view.data() == text );
		REQUIRE( view.size() == 4 );
	}

	SECTION( "string_view_t( const string_view_t &)" )
	{
		const char * text = "Test";
		string_view_t first{ text };
		string_view_t second{ first };

		REQUIRE( first.data() == second.data() );
		REQUIRE( first.size() == second.size() );
	}

}

TEST_CASE( "Test assignment operation" , "[string_view][assignment]" )
{
	string_view_t first{ "first" };
	string_view_t second{ "second" };

	first = second;

	REQUIRE( first.data() == second.data() );
	REQUIRE( first.size() == second.size() );
}

TEST_CASE( "Test iterators" , "[string_view][iterators]" )
{
	const char * text = "Test string";
	string_view_t view{ text };

	REQUIRE( view.begin() == text );
	REQUIRE( view.cbegin() == text );
	REQUIRE( view.end() == text + 11 );
	REQUIRE( view.cend() == text + 11 );
}

TEST_CASE( "Test for element access opertions" , "[string_view][element access]" )
{
	SECTION( "string_view_t::operator[]" )
	{
		const char * string = "String";
		string_view_t view{ string };

		REQUIRE( view[0] == 'S' );
		REQUIRE( view[1] == 't' );
		REQUIRE( view[2] == 'r' );
		REQUIRE( view[3] == 'i' );
		REQUIRE( view[4] == 'n' );
		REQUIRE( view[5] == 'g' );
	}

	SECTION( "string_view_t::at()" )
	{
		const char * string = "String";
		string_view_t view{ string };

		REQUIRE( view.at(0) == 'S' );
		REQUIRE( view.at(1) == 't' );
		REQUIRE( view.at(2) == 'r' );
		REQUIRE( view.at(3) == 'i' );
		REQUIRE( view.at(4) == 'n' );
		REQUIRE( view.at(5) == 'g' );

		REQUIRE_THROWS_AS( view.at(6), std::out_of_range );
	}

	SECTION( "string_view_t::front()" )
	{
		const char * string = "String";
		string_view_t view{ string };

		REQUIRE( view.front() == 'S' );
	}

	SECTION( "string_view_t::back()" )
	{
		const char * string = "String";
		string_view_t view{ string };

		REQUIRE( view.back() == 'g' );
	}

	SECTION( "string_view_t::data()" )
	{
		const char * string = "String";
		string_view_t view{ string };

		REQUIRE( view.data() == string );
	}
}

TEST_CASE( "Test for capacity opertions" , "[string_view][capacity]" )
{
	SECTION( "string_view_t::size()" )
	{
		const char * string = "String";
		string_view_t view{ string };

		REQUIRE( view.size() == 6 );
	}

	SECTION( "string_view_t::length()" )
	{
		const char * string = "String";
		string_view_t view{ string };

		REQUIRE( view.length() == 6 );
	}

	SECTION( "string_view_t::empty()" )
	{
		string_view_t view;

		REQUIRE( view.empty() );
	}
}

TEST_CASE( "Test for modifiers" , "[string_view][modifiers]" )
{
	const char * string = "abcdefghigklmnopqrstuvwxyz";
	string_view_t view{ string };

	REQUIRE( view.front() == 'a' );
	REQUIRE( view.back() == 'z' );
	REQUIRE( view.size() == 26 );

	view.remove_prefix(3);

	REQUIRE( view.front() == 'd' );
	REQUIRE( view.size() == 23 );

	view.remove_suffix(3);

	REQUIRE( view.back() == 'w' );
	REQUIRE( view.size() == 20 );

	const char * another_string = "hello";
	string_view_t another_view{ another_string };

	REQUIRE( another_view.data() == another_string );
	REQUIRE( another_view.size() == 5 );

	view.swap( another_view );

	REQUIRE( view.data() == another_string );
	REQUIRE( view.size() == 5 );

	REQUIRE( another_view.size() == 20 );
}

TEST_CASE( "Test for operations" , "[string_view][operations]" )
{
	SECTION( "string_view_t::copy()" )
	{
		string_view_t view = "abcdefghi";

		char buf[9];

		view.copy( buf, 9, 0 );

		REQUIRE( view == string_view_t(buf, 9) );
	}

	SECTION( "string_view_t::substr()" )
	{
		string_view_t original = "full string view";

		REQUIRE( original.substr() == original );
		REQUIRE( original.substr(5) == string_view_t("string view") );
		REQUIRE( original.substr(5, 6) == string_view_t("string") );
	}

	SECTION( "string_view_t::compare()" )
	{
		// compare( basic_string_view_t )

		string_view_t v1{"bcde"};
		string_view_t v2{"fghi"};
		string_view_t v3{"abcd"};
		string_view_t v4{"bcde"};
		string_view_t v5{"bcd"};
		string_view_t v6{"bcdef"};

		REQUIRE( v1.compare(v2) < 0 );
		REQUIRE( v1.compare(v3) > 0 );
		REQUIRE( v1.compare(v4) == 0 );
		REQUIRE( v1.compare(v5) > 0 );
		REQUIRE( v1.compare(v6) < 0 );
	}

	SECTION( "string_view_t::find()" )
	{
		string_view_t view = "text of string view with some text";

		// find( string_view_t, size_type )

		REQUIRE( view.find(string_view_t("text")) == 0 );
		REQUIRE( view.find(string_view_t("text"), 5) == 30 );

		// find( string_view_t, size_type )
		REQUIRE( view.find('o') == 5 );
	}

	SECTION( "string_view_t::rfind()" )
	{
		string_view_t view = "This is a string";

		static constexpr auto notfound = string_view_t::npos;

		REQUIRE( view.rfind("is") == 5 );
		REQUIRE( view.rfind("is", 4) == 2 );
		REQUIRE( view.rfind('s') == 10 );
		REQUIRE( view.rfind('q') == notfound );
	}

	SECTION( "string_view_t::find_first_of()" )
	{
		string_view_t view = "text of string view with some text";

		REQUIRE( view.find_first_of("is") == 8 );
		REQUIRE( view.find_first_of("os") == 5 );
	}

	SECTION( "string_view_t::find_last_of()" )
	{
		string_view_t view = "text of string view with some text";

		REQUIRE( view.find_last_of("is") == 25 );
		REQUIRE( view.find_last_of("os") == 26 );
	}

	SECTION( "string_view_t::find_first_not_of()" )
	{
		string_view_t view = "text of string view with some text";

		REQUIRE( view.find_first_not_of("extofs") == 4 );
		REQUIRE( view.find_first_not_of("tx") == 1 );
	}

	SECTION( "string_view_t::find_last_not_of()" )
	{
		string_view_t view = "text of string view with some text";

		REQUIRE( view.find_last_not_of("extofs") == 29 );
		REQUIRE( view.find_last_not_of("tx") == 31 );
	}
}

TEST_CASE( "Test for non-member functions" , "[string_view][non-member functions]" )
{
	string_view_t v1{"abc"};
	string_view_t v2{"def"};
	string_view_t v3{"ghi"};
	string_view_t v4{"abc"};

	REQUIRE( v1 < v2 );
	REQUIRE( v3 > v1 );
	REQUIRE( v1 == v4 );
	REQUIRE( v4 <= v2 );
	REQUIRE( v3 >= v4 );

	std::ostringstream ss;

	ss << v1;

	REQUIRE( ss.str() == "abc" );
}
