#include "HttpParser.hpp"

HttpRequest	HttpParser::parse(Client& client)
{
	HttpRequest		result;
	RequestMessage	request;
	
	request = client.getReq();

	result.requestMessage = request;
	if (request.uri.find("cgi-bin") == std::string::npos)
	{
		// When static
		result.is_static = true;
		result.path = request.uri;
		result.file_name = "./www" + result.path;
	}
	else
	{
		result.is_static = false;
		size_t	location = request.uri.find('?');
		if (location != std::string::npos)
			result.cgi_args = request.uri.substr(location + 1);
		result.file_name = "./www" + request.uri.substr(0, location);
	}
	return result;
};
