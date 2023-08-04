#include "HttpResponse.hpp"

std::vector<unsigned char>&	HttpResponse::build_response_message()
{
	std::stringstream	ss;

	// start-line
	ss << http_version << SPACE << status_code << SPACE << reason_phrase << CRLF;

	// header
	for (std::map<std::string, std::string>::iterator header = headers.begin(); header != headers.end(); ++header)
	{
		ss << header->first << COLON << SPACE << header->second << CRLF;
	}

	// body
	
	std::string	tmp_buffer = ss.str();
	std::vector<unsigned char>	buffer(tmp_buffer.begin(), tmp_buffer.end());
	return (buffer);
}
