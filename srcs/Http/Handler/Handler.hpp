#pragma once

#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <sys/stat.h>
#include "../HttpRequest.hpp"
#include "HttpStatusCodes.hpp"

# define CRLF "\r\n"
# define SERVER_HTTP_VERSION "HTTP/1.1"
# define SPACE " "
# define SERVER_NAME "webserv"
# define COLON ":"

class Handler
{
	protected:
		std::vector<unsigned char>			response;
		std::vector<unsigned char>			header;
		std::map<std::string, std::string >	headers;

		std::string					getFileType(std::string file_name);
		std::string					itos(int num);
		std::vector<unsigned char>	stou(std::stringstream& ss);
		void						buildHeader(int status_code);
	public:
		static bool	IsDirectory(std::string path);
		static bool	IsRegularFile(std::string path);
		static bool	IsFileReadble(std::string path);
		static bool	IsFileExist(std::string path);

		virtual std::vector<unsigned char>	handle(HttpRequest& request) = 0;
		virtual ~Handler() {}
};
