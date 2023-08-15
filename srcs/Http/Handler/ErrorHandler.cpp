#include "ErrorHandler.hpp"
#include "../../Client.hpp"

std::vector<unsigned char>	ErrorHandler::handle(Client &client, int status_code)
{
	std::vector<unsigned char>			body;
	std::map<std::string, std::string>	headers;
	std::string							file_name;
	bool								find_flag = false;

	// std::vector<int>::const_iterator it2;
	// std::map<std::vector<int>, std::string>::const_iterator it1 = client.getServer()->getErrorPage().begin();
	// for (; it1 != client.getServer()->getErrorPage().end(); it1++)
	// {
	// 	it2 = it1->first.begin();
	// 	for (; it2 != it1->first.end(); it2++)
	// 	{
	// 		std::cout << *it2 << std::endl;
	// 		if (*it2 == status_code)
	// 		{
	// 			std::cout << "FOUND DEFAULT" << std::endl;
	// 			find_flag = true;
	// 			break;
	// 		}
	// 	}
	// 	if (find_flag)
	// 		break;
	// }
	// if (find_flag)
	// 	file_name = it1->second;
	// else
	std::map<std::vector<int>, std::string> const& errorPage = client.getServer()->getErrorPage();
	std::map<std::vector<int>, std::string>::const_iterator it1 = errorPage.begin();
	for (;it1 != errorPage.end(); it1++)
	{
		for (std::vector<int>::const_iterator it2 = it1->first.begin(); it2 != it1->first.end(); it2++)
		{
			std::cout << "it2 : " << *it2 << " ";
		}
		std::cout << "it1 : " << it1->second << std::endl;
	}
		file_name = "./error_pages/40x.html";
	body = ReadStaticFile(file_name);
	headers["Connection"] = "close";
	headers["Content-Type"] = GetFileType(file_name);
	return BuildResponse(status_code, headers, body);
}

std::vector<unsigned char>	ErrorHandler::handle(Client& client)
{
	static_cast<void>(client);
}
