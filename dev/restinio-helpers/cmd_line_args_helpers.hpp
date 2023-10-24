/*
 * SObjectizer-5
 */

#pragma once

#include <restinio/impl/include_fmtlib.hpp>

#include <cstring>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <tuple>

namespace restinio_helpers
{

// Wrapper aroung one command-line argument value.
//
// It's intended to be used that way:
//
// ```cpp
// struct args_t
// {
// 	...
// 	int m_some_field{ 42 };
// 	...
// };
//
// args_t args;
// cmd_line_arg_t some_field_arg{ args.m_some_fields,
// 	"-s", "--some-fields",
// 	"some fields with default value: {}"
// };
// ... // Some parsing procedure.
// // At this point args.m_some_field isn't changed yet.
// some_field_arg.commit(); // Now args.m_some_field may receive a new value.
//
template< typename T >
struct cmd_line_arg_t
{
	// Reference to a field that should receive the result value.
	T & m_value_ref;

	// Value that may be receive as the result of command-line argument parsing.
	std::optional< T > m_value_taken;

	// Mandatory short name of the argument ("-a", for example).
	const char * m_short_name;
	// Mandatory long name of the argument ("--address", for example).
	const char * m_long_name;

	// Description of the argument to be passed as format string to fmt::format.
	const char * m_description_fmt_line;

	// NOTE: it's expected that all pointers are non-null.
	cmd_line_arg_t(
		T & value_ref,
		const char * short_name,
		const char * long_name,
		const char * description_fmt_line )
		:	m_value_ref{ value_ref }
		,	m_short_name{ short_name }
		,	m_long_name{ long_name }
		,	m_description_fmt_line{ description_fmt_line }
	{}

	// Should be called after parsing to move parsed value (if it's present)
	// into the target field.
	// If this method isn't called the parsed value will be lost.
	void
	commit()
	{
		if( m_value_taken.has_value() )
		{
			m_value_ref = std::move(*m_value_taken);
			m_value_taken.reset();
		}
	}
};

// Helper for printing description of one command-line argument.
//
// NOTE: it uses the fact that such usage of fmt::format:
//
// ```cpp
// fmt::format( "Argument is not used", argument );
// ```
//
// isn't an error.
//
template< typename T >
std::ostream &
operator<<( std::ostream & to, const cmd_line_arg_t< T > & what )
{
	to << "\n" << fmt::format(
			restinio::fmtlib_tools::runtime_format_string( "{:<30} {}" ),
			fmt::format(
					restinio::fmtlib_tools::runtime_format_string( "{}, {}" ),
					what.m_short_name,
					what.m_long_name ),
			fmt::format(
					restinio::fmtlib_tools::runtime_format_string(
							what.m_description_fmt_line ),
					what.m_value_ref ) );
	return to;
}

namespace process_cmd_line_args_impl
{

template< typename T >
[[nodiscard]]
bool
is_this_arg(
	const cmd_line_arg_t< T > & value_receiver,
	const char ** current )
{
	return (0 == std::strcmp( *current, value_receiver.m_short_name ) ||
			0 == std::strcmp( *current, value_receiver.m_long_name ));
}

// If current != last_arg then tries to parse value of an argument.
// If parse fails then an exception will be thrown.
// If current == last_arg then an exception will be thrown.
//
// If operation completes successfully then (current+1) is returned.
template< typename T >
const char **
take_and_shift_current(
	cmd_line_arg_t< T > & value_receiver,
	const char ** current,
	const char ** last_arg )
{
	const char ** prev = current;
	const char ** arg = ++current;

	if( arg == last_arg )
		throw std::runtime_error(
				std::string( "argument '" ) + *prev + "' requires value" );

	std::stringstream ss;
	ss << *arg;
	ss.seekg(0);

	T r;
	ss >> r;

	if( !ss || !ss.eof() )
		throw std::runtime_error(
				std::string( "unable to parse value for argument '" ) + *prev +
				"': '" + *arg + "'" );

	value_receiver.m_value_taken = std::move(r);

	return current;
}

// Special case for boolean argument: there is no need to extract
// a value for it (the presence of the argument sets the value to `true`).
//
// NOTE: returns `current` value (without incrementing it).
template<>
const char **
take_and_shift_current< bool >(
	cmd_line_arg_t< bool > & value_receiver,
	const char ** current,
	const char ** /*last_arg*/ )
{
	value_receiver.m_value_taken = true;

	return current;
}

//
// Helper that calls `is_this_arg` and (if necessary) `take_and_shift_current`
// for all (arg_head, arg_tail).
//
// Returns the updated value of `current` and a boolean flag.
// This flag is `true` if a value has been processed and extracted.
// This flag is `false` if the current command-line argument hasn't been processed.
//
template< typename Arg_Head, typename ...Args >
[[nodiscard]]
std::tuple< const char **, bool >
try_process_current_argument(
	const char ** current, const char ** last_arg,
	Arg_Head && arg_head,
	Args && ...arg_tail )
{
	if( is_this_arg( arg_head, current ) )
		return { take_and_shift_current( arg_head, current, last_arg ), true };
	else
	{
		if constexpr( 0 == sizeof...(arg_tail) )
			return { current, false };
		else
			return try_process_current_argument(
					current, last_arg,
					std::forward<Args>(arg_tail)... );
	}
}

} /* process_cmd_line_args_impl */

//
// It's expected that the Result type has the `m_help` field of type `bool`.
// This field will be set to `true` if `-h`/`--help` is specified. In that
// case other fields of `result` won't be changed.
//
template< typename Result, typename ...Args >
void
process_cmd_line_args(
	int argc,
	const char * argv[],
	Result & result,
	Args && ...args )
{
	using namespace process_cmd_line_args_impl;

	constexpr std::string_view arg_help_short{ "-h" };
	constexpr std::string_view arg_help_long{ "--help" };

	for( const char ** current = &argv[ 1 ], **last_arg = argv + argc;
			current != last_arg;
			++current )
	{
		bool current_value_processed{ false };

		if( *current == arg_help_short || *current == arg_help_long )
		{
			result.m_help = true;
			current_value_processed = true;
		}
		else
		{
			std::tie(current, current_value_processed) =
					try_process_current_argument( current, last_arg,
							std::forward<Args>(args)... );
		}

		if( !current_value_processed )
			throw std::runtime_error(
					std::string( "unknown argument: " ) + *current );
	}

	if( result.m_help )
	{
		std::cout << argv[ 0 ] << " [<options>]\n\noptions:\n";

		(std::cout << ... << args);

		std::cout << "\n\nOr -h, --help to show this message" << std::endl;
	}
	else
	{
		(args.commit(), ...);
	}
}

} /* namespace restinio_helpers */

