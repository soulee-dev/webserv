#include "ErrorHandler.hpp"

std::vector<unsigned char>	ErrorHandler::handler(int status_code)
{
	std::vector<unsigned char>			body;
	std::map<std::string, std::string>	headers;

	std::string	file_name = "./error_pages/404.html";
	body = ReadStaticFile(file_name);
	headers["Connection"] = "close";
	headers["Content-Type"] = GetFileType(file_name);
	return BuildResponse(status_code, headers, body);
}
