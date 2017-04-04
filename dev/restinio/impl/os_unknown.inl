inline std::tm
make_gmtime( std::time_t t )
{
	std::tm res = *gmtime( &t );

	return res;
}
