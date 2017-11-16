#include "route_parser.hpp"

namespace /* anonymous */
{

enum class parser_state_t : std::uint8_t
{
	wait_for_first_slash,
	wait_for_entity_first_char,
	wait_for_entity_id_or_new,
	wait_for_entity_id,
	wait_for_by_id_complete_or_avg_or_visits,
	wait_for_avg_or_visits_finish
};

} /* anonymous namespace */

route_parse_result_t
parse_route( const std::string & request_target )
{
	route_parse_result_t result;

	const int EOTARGET = 0x7FFFFFFF;
	std::size_t current_pos = 0;
	const std::size_t data_size = request_target.size();
	auto next_char = [&]() -> int {
		if( ++current_pos < data_size )
			return request_target[ current_pos ];
		return EOTARGET;
	};

	parser_state_t parser_internal_state = parser_state_t::wait_for_first_slash;
	int c = current_pos < data_size ? request_target[ current_pos ] : EOTARGET;

	auto on_successful_avg_or_visits_completion_req_type = request_type_t::unknown;

	while( true )
	{
		switch( parser_internal_state )
		{
			case parser_state_t::wait_for_first_slash:
				if( '/' == c )
					parser_internal_state = parser_state_t::wait_for_entity_first_char;
				else
					return result;

				break;
			/*case wait_for_first_slash:*/

			case parser_state_t::wait_for_entity_first_char:
				if( 'u'== c &&
					's' == next_char() &&
					'e' == next_char() &&
					'r'== next_char() &&
					's'== next_char() &&
					'/'== next_char() )
				{
					result.m_entity_type = entity_type_t::user;
					parser_internal_state = parser_state_t::wait_for_entity_id_or_new;
				}
				else if(
					'l' == c &&
					'o' == next_char() &&
					'c' == next_char() &&
					'a' == next_char() &&
					't' == next_char() &&
					'i' == next_char() &&
					'o' == next_char() &&
					'n'== next_char() &&
					's'== next_char() &&
					'/'== next_char() )
				{
					result.m_entity_type = entity_type_t::location;
					parser_internal_state = parser_state_t::wait_for_entity_id_or_new;
				}
				else if(
					'v' == c &&
					'i' == next_char() &&
					's' == next_char() &&
					'i' == next_char() &&
					't' == next_char() &&
					's'== next_char() &&
					'/'== next_char() )
				{
					result.m_entity_type = entity_type_t::visit;
					parser_internal_state = parser_state_t::wait_for_entity_id_or_new;
				}
				else { return result; }

				break;
			/*case wait_for_entity_first_char:*/

			case parser_state_t::wait_for_entity_id_or_new:
				if( 'n' == c )
				{
					if( 'e' == next_char() &&
						'w' == next_char() )
					{
						c = next_char();
						if( EOTARGET == c ||
							'?' == c ||
							('/' == c && EOTARGET == next_char() ) )
						{
							result.m_request_type = request_type_t::new_entity;
						}
					}

					return result;
				}
				else if( '0' > c || '9' < c )
				{
					return result;
				}

				// Here: digits only.
				// Fallthrough.
				parser_internal_state = parser_state_t::wait_for_entity_id;

			case parser_state_t::wait_for_entity_id:
				if( '0' <= c && '9' >= c )
				{
					result.m_id *= 10;
					result.m_id += c - '0';
				}
				else if( '/' == c )
				{
					parser_internal_state = parser_state_t::wait_for_by_id_complete_or_avg_or_visits;
				}
				else if( '?' == c || EOTARGET == c )
				{
					result.m_request_type = request_type_t::entity_by_id;
					return result;
				}
				else { return result; }

				break;

			/*case wait_for_entity_id_or_new:*/

			case parser_state_t::wait_for_by_id_complete_or_avg_or_visits:
				if( EOTARGET == c )
				{
					result.m_request_type = request_type_t::entity_by_id;
					return result;
				}
				else if(
					entity_type_t::location == result.m_entity_type &&
					'a' == c &&
					'v' == next_char() &&
					'g' == next_char() )
				{
					parser_internal_state = parser_state_t::wait_for_avg_or_visits_finish;
					on_successful_avg_or_visits_completion_req_type = request_type_t::location_avg;
				}
				else if(
					entity_type_t::user == result.m_entity_type &&
					'v' == c &&
					'i' == next_char() &&
					's' == next_char() &&
					'i' == next_char() &&
					't' == next_char() &&
					's' == next_char() )
				{
					parser_internal_state = parser_state_t::wait_for_avg_or_visits_finish;
					on_successful_avg_or_visits_completion_req_type = request_type_t::user_visits;
				}
				else { return result; }

				break;
			/*case wait_for_by_id_complete_or_avg_or_visits:*/

			case parser_state_t::wait_for_avg_or_visits_finish:
				if( EOTARGET == c ||
					'?' == c ||
					('/' == c && EOTARGET == next_char() ) )
				{
					result.m_request_type = on_successful_avg_or_visits_completion_req_type;
				}

				return result;
			/*case wait_for_avg_or_visits_finish:*/
		}

		c = next_char();
	}

	return result;
}
