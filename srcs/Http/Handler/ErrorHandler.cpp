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
		file_name = it->second;
	else
		file_name = "./error_pages/50x.html";
	body = ReadStaticFile(file_name);
	headers["Connection"] = "close";
	headers["Content-Type"] = GetFileType(file_name);
	return BuildResponse(status_code, headers, body);
}

std::vector<unsigned char>	ErrorHandler::handle(Client& client)
{
	static_cast<void>(client);
}
