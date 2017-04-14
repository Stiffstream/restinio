/*
	restinio
*/

/*!
	Restinio common types.
*/


namespace restinio
{

//! Request id in scope of single connection.
using request_id_t = unsigned int;

//! Response output flags for buffers commited to response-coordinator
enum class response_output_flags_t
{
	response_complete = 0x01,
	connection_close = 0x02
};

} /* namespace restinio */
