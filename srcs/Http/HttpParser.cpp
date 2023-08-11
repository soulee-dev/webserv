#include "HttpParser.hpp"

HttpRequest	HttpParser::parse(Client& client, std::vector<std::string> List)
{
	HttpRequest		result;
	RequestMessage	request; 
	request = client.getFrontReq();
	
	std::istringstream	iss(List.back());
	List.pop_back();
	iss >> result.isAutoIndex;
	result.root = List.back();
	List.pop_back();
	result.method = request.method;
	std::cout << BOLDMAGENTA << "METHOD : " << result.method << '\n';
	
	result.indexList = List;
	result.errnum = 0;

	if (request.uri.find("cgi-bin") == std::string::npos)
	{
		// When static
		result.is_static = true;
		result.path = request.uri;
		std::cout << BOLDYELLOW << "URI(PATH) : " << request.uri << '\n';
		result.file_name = result.root; // + result.path;
		std::cout << BOLDGREEN << "FILE NAME : " << result.file_name << '\n';
	}
	else
	{
		result.is_static = false;
		size_t	location = request.uri.find('?');
		if (location != std::string::npos)
			result.cgi_args = request.uri.substr(location + 1);
		result.file_name = "." + request.uri.substr(0, location);
		result.body = request.body;
	}
	return result;
};
