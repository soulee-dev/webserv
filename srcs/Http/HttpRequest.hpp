#pragma once

#include <string>
#include <vector>

# define CRLF "\r\n"

class HttpRequest
{
	public:
		bool		is_static;
		std::string	file_name;
		std::string	path;
		off_t		file_size;
		std::string	cgi_args;

		// gyopark ADDED
		std::string	header;
		std::vector<unsigned char> ubuffer;
		std::vector<unsigned char> body;
};
