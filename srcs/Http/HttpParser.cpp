#include "HttpParser.hpp"

HttpRequest	HttpParser::parse(Client& client, std::vector<std::string> List, int block)
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
	std::cout << "BLOCK : " << block << RESET << '\n';
	std::cout << "block 1이면 conf 파일보다 검색한 문장을 우선으로 파싱합니다. block 0이면 default.conf 참조합니다.\n";
	
	if (request.requestTarget.find("cgi-bin") == std::string::npos)
	{
		// When static
		result.is_static = true;
		result.path = request.requestTarget;
		std::cout << BOLDYELLOW << "URI(PATH) : " << request.requestTarget << '\n';
		std::cout << BOLDYELLOW << "ROOT : " << result.root << '\n';
		if (block == 1)
			result.file_name = result.root + List[0]; // + result.path;
		else
			result.file_name = result.root;
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
