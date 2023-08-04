#pragma once

#include <string>

class HttpRequest
{
	public:
		bool		is_static;
		std::string	file_name;
		std::string	path;
		off_t		file_size;
		std::string	cgi_args;
};
