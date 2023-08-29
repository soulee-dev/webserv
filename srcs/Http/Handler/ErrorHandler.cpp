#include "ErrorHandler.hpp"
#include "../../Client.hpp"
#include <sys/stat.h>

static std::string intToString(int number)
{
	std::stringstream sstream;
	sstream << number;
	return sstream.str();
}

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
	{
		std::cout << intToString(status_code);
		file_name = "./html/error_pages/" + intToString(status_code) + ".html";
		struct stat error_page_stat;
		if (stat(file_name.c_str(), &error_page_stat) != 0 || !S_ISREG(error_page_stat.st_mode))
			file_name = "./html/error_pages/40x.html";
	}

	// TODO check whether there is file
	client.response.status_code = status_code;
	client.response.headers["Connection"] = "close";
	client.response.headers["Content-Type"] = GetFileType(file_name);
	ReadStaticFile(client, file_name);
}
