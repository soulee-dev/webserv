#pragma once

#include <string>
#include <map>
#include <vector>
#include <sstream>

# define CRLF "\r\n";

class HttpResponse
{
	private:
		// start-line
		std::string							http_version;
		int									status_code;
		std::string							reason_phrase;

		// header
		std::map<std::string, std::string>	headers;

		// body
		std::vector<unsigned char>			body;

	public:
		static std::vector<unsigned char>&	build_response_message();
};
