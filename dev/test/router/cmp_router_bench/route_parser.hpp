#pragma once

#include <string>

enum class entity_type_t : std::uint8_t
{
	user,
	location,
	visit,
	unknown
};

enum class request_type_t : std::uint8_t
{
	entity_by_id, /*Get entity or update entity*/
	new_entity,
	user_visits,
	location_avg,
	unknown
};

//
// route_parse_result_t
//

struct route_parse_result_t
{
	entity_type_t m_entity_type{ entity_type_t::unknown };
	request_type_t m_request_type{ request_type_t::unknown };

	std::uint32_t m_id{0};
};

route_parse_result_t
parse_route( const std::string & request_target );
