#pragma once

#include <map>
#include <vector>
#include <sstream>
#include "../HttpRequest.hpp"

# define CRLF "\r\n"
# define HTTP_VERSION "HTTP/1.1"
# define SPACE " "
# define SERVER_NAME "webserv"

class Handler
{
	private:
		std::vector<unsigned char>	ssToUc(std::stringstream& ss);
		std::stringstream			buildHandle(int status_code, std::map<std::string, std::string>& headers);
	public:
		virtual std::vector<unsigned char>	handle(HttpRequest& request) const = 0;
};
