#include "../Location.hpp"
#include "HttpRequestManager.hpp"

HttpRequestManager::HttpRequestManager(Client& client)
{
	Location		location;

	request = parser.parse(client);
	location = client.getServer()->getLocations().find("/")->second;

	// 맞지 않는 location 들어오면 에러처리
	// if Static
	if (request.is_static)
	{	
		if (Handler::IsDirectory(request.file_name))
		{
			for (std::vector<std::string>::const_iterator index = location.getIndex().begin(); index != location.getIndex().end(); ++index)
			{
				struct stat	index_stat_buffer;
				std::string	path = request.file_name + *index;
				if (Handler::IsRegularFile(path) && Handler::IsFileReadble(path))
				{
					request.file_name = path;
					handler = new StaticHandler();
					return ;
				}
			}
			if (location.getAutoIndex())
			{
				std::cout << "Auto Index" << std::endl;
				return ;
			}
		}
		else
		{
			if (!Handler::IsFileExist(request.file_name))
			{
				request.status_code = 404;
				request.long_msg = "Webserv couldn't find this file";
				handler = new ErrrorHandler();
				return ;
			}
			if (!Handler::IsRegularFile(request.file_name) || !Handler::IsFileReadble(request.file_name))
			{
				request.status_code = 403;
				request.long_msg = "Webserv couldn't read the file";
				handler = new ErrrorHandler();
				return ;
			}
			handler = new StaticHandler();
		}
	}
	else
	{
		handler = new DynamicHandler();
	}
}

std::vector<unsigned char>	HttpRequestManager::processRequest(void)
{
	return handler->handle(request);
}

HttpRequestManager::~HttpRequestManager()
{
	delete handler;
}
