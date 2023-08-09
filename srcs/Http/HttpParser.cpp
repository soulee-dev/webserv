#include "HttpParser.hpp"

HttpRequest	HttpParser::parse(Client& client, std::vector<std::string> List)
{
	HttpRequest		result;
	RequestMessage	request; 
	request = client.getReq();
	
	std::istringstream	iss(List.back());
	List.pop_back();
	iss >> result.isAutoIndex;
	result.root = List.back();
	List.pop_back();
	result.method = request.method;
	std::cout << BOLDMAGENTA << "METHOD : " << result.method << '\n';
	
	result.indexList = List;
	result.errnum = 0;

	if (request.requestTarget.find("cgi-bin") == std::string::npos)
	{
		// When static
		result.is_static = true;
		result.path = request.requestTarget;
		std::cout << BOLDYELLOW << "URI(PATH) : " << request.requestTarget << '\n';
		result.file_name = result.root; // + result.path;
		std::cout << BOLDGREEN << "FILE NAME : " << result.file_name << '\n';
	}
	else
	{
		result.is_static = false;
		size_t	location = request.requestTarget.find('?');
		if (location != std::string::npos)
			result.cgi_args = request.requestTarget.substr(location + 1);
		result.file_name = "." + request.requestTarget.substr(0, location);
		result.body = request.body;
	}
	return result;
};
