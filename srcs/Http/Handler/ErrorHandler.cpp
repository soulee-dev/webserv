#include "ErrorHandler.hpp"
#include "../../Client.hpp"

std::vector<unsigned char>	ErrorHandler::handler(Client& client, int status_code)
{
	std::vector<unsigned char>			body;
	std::map<std::string, std::string>	headers;
	std::string							file_name;
	std::map<std::vector<int>, std::string> const& errorPage = client.getServer()->getErrorPage();
	bool								find_flag = false;

	std::map<std::vector<int>, std::string>::const_iterator it = errorPage.begin();
	for (; it != errorPage.end(); it++)
	{
		std::vector<int>::const_iterator it2 = it->first.begin();
		for (; it2 != it->first.end(); it2++)
		{
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
		file_name = it->second;
	else
		file_name = "./error_pages/404.html";
	body = ReadStaticFile(file_name);
	headers["Connection"] = "close";
	headers["Content-Type"] = GetFileType(file_name);
	return BuildResponse(status_code, headers, body);
}

ErrorHandler::ErrorHandler(Client const& client) : Handler(client)
{
}