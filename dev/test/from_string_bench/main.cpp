/*
	restinio
*/

/*!
	Benchmark for to_lower approaches.
*/

#include <iostream>
#include <string>
#include <vector>
#include <chrono>

#include <boost/lexical_cast.hpp>

#include <restinio/utils/from_string.hpp>

template < typename LAMBDA >
void
run_bench( const std::string & tag, LAMBDA lambda )
{
	try
	{
		auto started_at = std::chrono::high_resolution_clock::now();
		lambda();
		auto finished_at = std::chrono::high_resolution_clock::now();
		const double duration =
			std::chrono::duration_cast< std::chrono::microseconds >(
				finished_at - started_at ).count() / 1000.0;

		std::cout << "Done '" << tag << "': " << duration << " ms" << std::endl;
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Failed to run '" << tag << "': " << ex.what() << std::endl;
	}
}

template < typename Integer >
struct cast_pair_t
{
	cast_pair_t() {}
	cast_pair_t(
		std::string str_,
		Integer int_)
		:	m_str{ std::move( str_ ) }
		,	m_int{ int_ }
	{}

	std::string m_str;
	Integer m_int;
};

template < typename Integer >
using cast_dataset_t = std::vector< cast_pair_t< Integer > >;

template < typename Integer >
auto
create_ints( std::size_t n )
{
	cast_dataset_t< Integer > result;
	while( n-- )
	{
		int r[2];
		r[0] = std::rand();
		Integer i;

		if( sizeof( i ) > sizeof( r ) )
			r[1] = std::rand();

		memcpy( &i, r, sizeof( i ) );

		result.emplace_back( std::to_string( i ), i );

		if( 0 <= i )
			result.emplace_back( "+" + std::to_string( i ), i );
	}

	return result;
}

cast_dataset_t< std::int8_t > ints8_data;
cast_dataset_t< std::uint8_t > uints8_data;

cast_dataset_t< std::int16_t > ints16_data;
cast_dataset_t< std::uint16_t > uints16_data;

cast_dataset_t< std::int32_t > ints32_data;
cast_dataset_t< std::uint32_t > uints32_data;

cast_dataset_t< std::int64_t > ints64_data;
cast_dataset_t< std::uint64_t > uints64_data;

void
init_datasets( size_t n )
{
	std::srand( std::time( nullptr ) );

	ints8_data = create_ints< std::int8_t >( n );
	uints8_data = create_ints< std::uint8_t >( n );
	ints16_data = create_ints< std::int16_t >( n );
	uints16_data = create_ints< std::uint16_t >( n );
	ints32_data = create_ints< std::int32_t >( n );
	uints32_data = create_ints< std::uint32_t >( n );
	ints64_data = create_ints< std::int64_t >( n );
	uints64_data = create_ints< std::uint64_t >( n );
}

#define RUN_CAST_BENCH( bench_func, tag ) \
	run_bench( \
		tag, \
		[&]{ bench_func( str_set_1, str_set_2 ); } );

constexpr std::size_t iterations = 1000;

template < typename Integer >
void
bench_intN( const cast_dataset_t< Integer > & data )
{
	for( int i = 0; i < iterations; ++i )
	{
		for( const auto & p : data )
			if( p.m_int != restinio::utils::from_string< Integer >( p.m_str ) );
	}
}

void bench_int8(){ bench_intN( ints8_data ); }
void bench_uint8(){ bench_intN( uints8_data ); }
void bench_int16(){ bench_intN( ints16_data ); }
void bench_uint16(){ bench_intN( uints16_data ); }
void bench_int32(){ bench_intN( ints32_data ); }
void bench_uint32(){ bench_intN( uints32_data ); }
void bench_int64(){ bench_intN( ints64_data ); }
void bench_uint64(){ bench_intN( uints64_data ); }

template < typename Integer >
void
boost_bench_intN( const cast_dataset_t< Integer > & data )
{
	for( int i = 0; i < iterations; ++i )
	{
		for( const auto & p : data )
			if( p.m_int != boost::lexical_cast< Integer >( p.m_str ) );
	}
}

void boost_bench_int8(){ boost_bench_intN( ints8_data ); }
void boost_bench_uint8(){ boost_bench_intN( uints8_data ); }
void boost_bench_int16(){ boost_bench_intN( ints16_data ); }
void boost_bench_uint16(){ boost_bench_intN( uints16_data ); }
void boost_bench_int32(){ boost_bench_intN( ints32_data ); }
void boost_bench_uint32(){ boost_bench_intN( uints32_data ); }
void boost_bench_int64(){ boost_bench_intN( ints64_data ); }
void boost_bench_uint64(){ boost_bench_intN( uints64_data ); }


int
main()
{
	init_datasets( 1000 );

	run_bench( "int8", bench_int8 );
	run_bench( "uint8", bench_uint8 );
	run_bench( "int16", bench_int16 );
	run_bench( "uint16", bench_uint16 );
	run_bench( "int32", bench_int32 );
	run_bench( "uint32", bench_uint32 );
	run_bench( "int64", bench_int64 );
	run_bench( "uint64", bench_uint64 );

	// run_bench( "boost int8", boost_bench_int8 );
	// run_bench( "boost uint8", boost_bench_uint8 );
	run_bench( "boost int16", boost_bench_int16 );
	run_bench( "boost uint16", boost_bench_uint16 );
	run_bench( "boost int32", boost_bench_int32 );
	run_bench( "boost uint32", boost_bench_uint32 );
	run_bench( "boost int64", boost_bench_int64 );
	run_bench( "boost uint64", boost_bench_uint64 );

	return 0;
}
