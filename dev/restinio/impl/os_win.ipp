inline std::tm
make_gmtime( std::time_t t )
{
	std::tm res;

	gmtime_s( &res, &t );

	return res;
}

inline std::tm
make_localtime( std::time_t t )
{
	std::tm res;

	localtime_s( &res, &t );

	return res;
}
