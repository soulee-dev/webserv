#pragma once

#include "Message.hpp"
#include "../Color.hpp"
#include "../Location.hpp"
#include <iostream>
#include <sstream>

enum ParseErrorCode
{
	NOT_ERROR = 0,
    BAD_REQUEST = 400,
	METHOD_NOT_ALLOWED = 405,
    HTTP_VERSION_NOT_SUPPORT = 505,
};

class Request : public Message
{
	public:
		Request(void);
		int	file_fd;
		size_t file_size;
		size_t RW_file_size;
		size_t			writeIndex;
		bool		is_static;
		bool		is_put;
		bool		is_post_dynamic;
		std::string	file_name;
		std::string	path;
		std::string	cgi_args;
		std::string	cgi_path_info;
		int pipe_fd[2], pipe_fd_back[2];
		Location	location;
		std::string	location_uri;
    	std::string uri;
		enum ParseErrorCode errorCode;

		void	clear(void);
};
