#include "HttpRequestManager.hpp"
#include "../Client.hpp"
#include "../Location.hpp"

HttpRequestManager::HttpRequestManager() : handler(NULL)
{
}

void	HttpRequestManager::setHandler(Client& client)
{
	parse(client);
	if (getFrontReq().is_static)
	{
		std::cout << BOLDRED << " -- PROCESSING STATIC -- \n";
		handler = new StaticHandler();
	}
	else
	{
		std::cout << BOLDBLUE << " -- PROCESSING DYNAMIC --\n";
		handler = new DynamicHandler();
	}
}

void	HttpRequestManager::parse(Client& client)
{
	HttpRequest&	request = getFrontReq();
	bool			is_found;
	size_t			location_pos;
	std::string		found_uri;
	std::string		tmp_uri;
	std::map<std::string, Location> locations = client.getServer()->getLocations();
	std::map<std::string, Location>::iterator location;
                                                                                                  
	if (request.uri[request.uri.size() -1] != '/')
		request.uri += "/";
	tmp_uri = request.uri;
	while (tmp_uri != "/")
	{
		if (is_found)
			break ;
		location_pos = tmp_uri.find_last_of('/');
		if (location_pos == std::string::npos)
			break;
		if (location_pos == 0)
			tmp_uri = "/";
		else
			tmp_uri = std::string(tmp_uri.begin(), tmp_uri.begin() + location_pos);
		for (location = locations.begin(); location != locations.end(); ++location)
		{
			if (tmp_uri == location->first)
			{
				std::cout << "LOCATION: " << location->first << std::endl; 
				found_uri = location->first;
				is_found = true;
				request.location = client.getServer()->getLocations()[found_uri];
				break;
			}
		}
	}
	if (is_found)
	{
		request.file_name = request.uri.substr(location_pos + 1);
		// Remove last character slash
		request.file_name.erase(request.file_name.size() - 1);
	}
	if (request.uri.find("cgi-bin") == std::string::npos)
	{
		std::string	fileName;
		request.is_static = true;
		// request.path = request.location.getRoot() + ;
		std::cout << BOLDYELLOW << "URI(PATH) : " << request.uri << '\n';
		std::cout << BOLDGREEN << "FILE NAME : " << request.file_name << '\n';
	}
	else
	{
		request.is_static = false;
		size_t	pos = request.uri.find('?');
		if (pos != std::string::npos)
			request.cgi_args = request.uri.substr(pos + 1);
		request.file_name = "." + request.file_name.substr(0, pos);
	}
};

HttpRequest& HttpRequestManager::getBackReq(void)
{
	return queReq.back();
}

HttpRequest& HttpRequestManager::getFrontReq(void)
{
	return queReq.front();
}

HttpRequest HttpRequestManager::popReq(void)
{
    HttpRequest	ret = queReq.front();
    queReq.pop();
    return ret;
}

std::vector<unsigned char>	HttpRequestManager::processRequest(Client& client)
{
	return handler->handle(client);
}

HttpRequest& HttpRequestManager::getRequest()
{
	return getFrontReq();
}

HttpRequestManager::~HttpRequestManager()
{
	delete handler;
}

void	HttpRequestManager::pushReq()
{
	queReq.push(HttpRequest());
}

void HttpRequestManager::dynamicOpenFd(Client& client)
{
	DynamicHandler *currHandler = dynamic_cast<DynamicHandler*>(handler);

	if (currHandler != NULL)
		currHandler->openFd(client);
}

void HttpRequestManager::sendReqtoEvent(Client& client)
{
	DynamicHandler *currHandler = dynamic_cast<DynamicHandler*>(handler);

	if (currHandler != NULL)
		currHandler->sendReqtoCgi(client);
	else
	{
		StaticHandler *currHandler = dynamic_cast<StaticHandler*>(handler);
		currHandler->sendReqtoEvent(client);
	}
}

void HttpRequestManager::dynamicRunCgi(Client& client)
{
	DynamicHandler *currHandler = dynamic_cast<DynamicHandler*>(handler);

	if (currHandler != NULL)
		currHandler->runCgi(client);
}

void HttpRequestManager::dynamicMakeResponse(Client& client)
{
	DynamicHandler *currHandler = dynamic_cast<DynamicHandler*>(handler);

	if (currHandler != NULL)
		currHandler->makeResponse(client);
}

void HttpRequestManager::dynamicReadFromCgi(Client& client)
{
	DynamicHandler *currHandler = dynamic_cast<DynamicHandler*>(handler);

	if (currHandler != NULL)
		currHandler->readFromCgi(client);
}
