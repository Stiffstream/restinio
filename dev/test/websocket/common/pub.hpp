#pragma once

#include <string>
#include <vector>

inline std::string
to_char_each( std::vector< int > source )
{
	std::string result;
	result.reserve( source.size() );

	for( const auto & val : source )
	{
		result.push_back( static_cast<char>(val) );
	}

	return result;
}

