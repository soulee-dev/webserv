#include "Handler.hpp"



std::vector<unsigned char>	ssToUc(std::stringstream& ss)
{
	std::string					tmp_string = ss.str();
	std::vector<unsigned char>	result(tmp_string.begin(), tmp_string.end());
}

std::stringstream	Handler::buildHandle(int status_code, std::map<std::string, std::string>& headers)
{
	std::stringstream			ss;

	// start-line
	ss << HTTP_VERSION << SPACE << status_code << CRLF;

	for (std::map<std::string, std::string>::iterator header = headers.begin(); header != headers.end(); ++header)
	{
		ss << header->first << SPACE << header->second << CRLF;
	}
	ss << CRLF;

	return (ss);
}
