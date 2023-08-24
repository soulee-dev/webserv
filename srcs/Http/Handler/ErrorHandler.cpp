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
	for (; it != errorPage.end(); ++it)
	{
		std::vector<int>::const_iterator it2 = it->first.begin();
		for (; it2 != it->first.end(); ++it2)
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
		file_name = "./html/error_pages/" + it->second;
	else
		file_name = "./html/error_pages/40x.html";
	try
	{
		body = ReadStaticFile(file_name);
	}
	catch (const std::length_error& e)
	{
		std::cout << "Can't found erorr page!!" << std::endl;
	}
	headers["Connection"] = "close";
	headers["Content-Type"] = GetFileType(file_name);
	return BuildResponse(status_code, headers, body);
}

void ErrorHandler::sendReqtoError(Client &client)
{
	int errCode = client.request.errorCode;
	client.sendBuffer = ErrorHandler::handle(client, errCode);
}
std::vector<unsigned char>	ErrorHandler::handle(Client& client) const
{
	std::vector<unsigned char>	a;

	static_cast<void>(client);
	return a;
}