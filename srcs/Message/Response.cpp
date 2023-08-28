#include "Response.hpp"
#include <cstring>

Response::Response()
{
	clear();
}

void Response::clear(void)
{
	Message::clear();

	is_auto_index = false;
}
