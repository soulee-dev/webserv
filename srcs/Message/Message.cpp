#include "Message.hpp"

void	Message::clear(void)
{
	status_code = 0;
	method = "";
	http_version = "";
	headers.clear();
	body.clear();
}
