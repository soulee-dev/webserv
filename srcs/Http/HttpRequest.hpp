#pragma once

#include <string>
#include <vector>

#include "../Color.hpp"
#include <iostream>
#include <sstream>
#include <map>

enum ParseErrorCode
{
    BAD_REQUEST = 400,
    HTTP_VERSION_NOT_SUPPORT = 505,
};

# define CRLF "\r\n"
class HttpRequest
{
	public:
	    std::vector<unsigned char> raw;
    	std::string startLine;
    	std::string httpVersion;
		bool		is_static;
		std::string	file_name;
		std::string	path;
		off_t		file_size;
		std::string	cgi_args;

    	std::string uri;
		enum ParseErrorCode errorCode;
		// ------- gyopark ADDED ------- //
		std::string	method;
		std::string	header;
		std::map<std::string, std::string> headers;
		std::vector<unsigned char> ubuffer;
		std::vector<unsigned char> body;
		std::vector<std::string> indexList;
		std::string	root;
		std::string target;
		bool	isAutoIndex;
		int		errnum;
		int		check; // 파일이 존재하면서 동시에 autoindex on인 경우를 확인하기 위한 변수
		// ----------------------------- //
};
