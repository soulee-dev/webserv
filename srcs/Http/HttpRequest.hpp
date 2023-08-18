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

enum ErrorCode
{
  INTERNAL_SERVER_ERROR = 500,
};

# define CRLF "\r\n"
class HttpRequest
{
	public:
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
		// enum ParseErrorCode errorCode; //이거 왜? enum ParseErrorCode type 일까용? 
        int         errorCode;
		std::string	method;
		std::map<std::string, std::string> headers;
		std::vector<unsigned char> body;
};
