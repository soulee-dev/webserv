#include "ErrorHandler.hpp"
#include "../../Client.hpp"

std::vector<unsigned char>	ErrorHandler::handle(Client &client, int status_code)
{
	std::vector<unsigned char>			body;
	std::map<std::string, std::string>	headers;
	std::string							file_name;
	bool								find_flag = false;

	std::map<std::vector<int>, std::string> const& errorPage = client.getServer()->getErrorPage();
	std::map<std::vector<int>, std::string>::const_iterator it = errorPage.begin();
	for (; it != errorPage.end(); it++)
	{
		std::vector<int>::const_iterator it2 = it->first.begin();
		for (; it2 != it->first.end(); it2++)
		{
			std::cout << "it2 : " << *it2 << std::endl;
			if (status_code == *it2)
			{
				find_flag = true;
				break ;
			}
		}
		if (find_flag)
			break ;
	}
	if (find_flag)
	{
		std::cout << "찾았어요!" << std::endl;
		file_name = "./error_pages/" + it->second;
	}
	else
	{
		std::cout << "못찾았으니 디폴트" << std::endl;
		file_name = "./error_pages/40x.html";
	}
	try { // 존재하지 않는 파일을 ReadStaticFile을 시도했을때 에러처리
		body = ReadStaticFile(file_name);
	} catch (const std::length_error& e) {
		std::cout << "Can't found erorr page!!" << std::endl;
	}
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
	client.sendBuffer = ErrorHandler::handle(client, err);
}

std::vector<unsigned char>	ErrorHandler::handle(Client& client)
{
	static_cast<void>(client);
	return std::vector<unsigned char>();
}
