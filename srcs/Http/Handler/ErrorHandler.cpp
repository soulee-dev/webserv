#include "ErrorHandler.hpp"
#include "../../Client.hpp"

std::vector<unsigned char>	ErrorHandler::handler(int status_code)
{
	std::vector<unsigned char>			body;
	std::map<std::string, std::string>	headers;

	// TODO: Config에서 맞게 읽어도록 하기
	std::string	file_name = find_error_page(status_code);
	body = ReadStaticFile(file_name);
	headers["Connection"] = "close";
	headers["Content-Type"] = GetFileType(file_name);
	return BuildResponse(status_code, headers, body);
}

std::string find_error_page(int status_code)
{
	std::string file_name;
	switch (status_code)
	{
		case 400: // Bad Request
			file_name = "./error_pages/400.html";
			break ;
		case 403: // Forbidden
			file_name = "./error_pages/403.html";
			break ;
		case 404: // Not Found
			file_name = "./error_pages/404.html";
			break ;
		case 405: // Method Not Allowed
			file_name = "./error_pages/405.html";
			break ;
		case 505: // HTTP Version Not Supported
			file_name = "./error_pages/505.html";
			break ;	
	}
	return file_name;
}

void ErrorHandler::sendReqtoError(Client &client)
{
	int err = client.httpRequestManager.getRequest().errorCode;
	client.sendBuffer = ErrorHandler::handler(err);
}
