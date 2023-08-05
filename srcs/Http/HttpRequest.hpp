#pragma once

#include <string>

class HttpRequest
{
	public:
		// Error handler
		int			status_code;
		std::string	long_msg;

		std::string	method;
		bool		is_static;
		std::string	file_name;
		std::string	path;
		off_t		file_size;
		std::string	cgi_args;
};
