#pragma once

#include <string>
#include <vector>

#include "../Color.hpp"
#include "../Location.hpp"
#include <iostream>
#include <sstream>
#include <map>

enum ParseErrorCode
{
    BAD_REQUEST = 400,
	METHOD_NOT_ALLOWED = 405,
    HTTP_VERSION_NOT_SUPPORT = 505,
};

# define CRLF "\r\n"
class HttpRequest
{
	public:
	    std::vector<unsigned char> raw;
    	std::string startLine;
    	std::string	httpVersion;
		bool		is_static;
		std::string	file_name;
		std::string	path;
		std::string	cgi_args;
		std::string	cgi_path_info;
		int pipe_fd[2], pipe_fd_back[2];
		Location	location;
		std::string	location_uri;
    	std::string uri;
		enum ParseErrorCode errorCode;
		std::string	method;
		std::string	header;
		std::map<std::string, std::string> headers;
		std::vector<unsigned char> body;
};
