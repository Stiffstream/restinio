
std::string
create_random_text( std::size_t n, std::size_t repeat_max = 1 )
{
	restinio::string_view_t symbols{
		// " \t\r\n"
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		// ",.;:'\"!@#$%^&*~-+\\/"
		"<>{}()"};

	std::string result;
	result.reserve( n );

	while( 0 != n )
	{
		const char c = symbols[ std::rand() % symbols.size() ];
		std::size_t repeats = 1 + std::min< std::size_t >( n - 1, std::rand() % repeat_max );
		result.append( repeats, c );
		n -= repeats;
	}

	return result;
}

std::string
create_random_binary( std::size_t n, std::size_t repeat_max = 1 )
{
	std::string result;
	result.reserve( n );

	while( 0 != n )
	{
		const unsigned char c = std::rand() % 256;
		std::size_t repeats = 1 + std::min< std::size_t >( n - 1, std::rand() % repeat_max );
		result.append( repeats, c );
		n -= repeats;
	}

	return result;
}
