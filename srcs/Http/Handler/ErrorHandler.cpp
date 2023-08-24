#include "ErrorHandler.hpp"
#include "../../Client.hpp"

void	HandleError(Client &client, int status_code)
{
	std::vector<unsigned char>			body;
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

	// TODO check whether there is file
	client.response.status_code = status_code;
	client.response.headers["Connection"] = "close";
	client.response.headers["Content-Type"] = GetFileType(file_name);
	ReadStaticFile(client, file_name);
}
