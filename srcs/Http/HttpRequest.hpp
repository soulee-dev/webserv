#pragma once

#include <string>
#include <vector>

#include "../Color.hpp"
#include <iostream>
#include <sstream>

# define CRLF "\r\n"

// extern std::string	prevString; 

class HttpRequest
{
	public:
		bool		is_static;
		std::string	file_name;
		std::string	path;
		off_t		file_size;
		std::string	cgi_args;

		// ------- gyopark ADDED ------- //
		std::string	method;
		std::string	header;
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
