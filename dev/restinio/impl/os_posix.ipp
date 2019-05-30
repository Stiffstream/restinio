inline std::tm
make_gmtime( std::time_t t )
{
	std::tm res;

	gmtime_r( &t, &res );

	return res;
}

inline std::tm
make_localtime( std::time_t t )
{
	std::tm res;

	localtime_r( &t, &res );

	return res;
}
