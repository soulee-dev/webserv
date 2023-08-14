#include "HttpRequestManager.hpp"
#include "../Client.hpp"
#include "../Location.hpp"

HttpRequestManager::HttpRequestManager() : handler(NULL)
{
}

void	HttpRequestManager::SetHandler(Client& client)
{
	Parse(client);
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

void	HttpRequestManager::Parse(Client& client)
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
				found_uri = location->first;
				is_found = true;
				break;
			}
		}
	}
	if (is_found)
	{
		request.file_name = request.uri.substr(location_pos);
		// Remove last character slash
		request.file_name.erase(request.file_name.size() - 1);
	}
	else
		found_uri = "/";
	std::cout << "LOCATION: " << found_uri << std::endl; 
	if (request.uri.find("cgi-bin") == std::string::npos)
	{
		std::string	fileName;
		request.is_static = true;
	}
	else
	{
		request.is_static = false;
		size_t	pos = request.uri.find('?');
		if (pos != std::string::npos)
		{
			request.cgi_args = request.uri.substr(pos + 1);
			request.cgi_args = request.cgi_args.erase(request.cgi_args.size() - 1);
		}
		request.file_name = request.file_name.substr(0, pos);
	}
	request.location_uri = found_uri;
	request.location = client.getServer()->getLocations()[found_uri];
	request.path = request.location.getRoot() + request.file_name;
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

void HttpRequestManager::DynamicOpenFd(Client& client)
{
	DynamicHandler *currHandler = dynamic_cast<DynamicHandler*>(handler);

	if (currHandler != NULL)
		currHandler->OpenFd(client);
}

void HttpRequestManager::SendReqtoEvent(Client& client)
{
	DynamicHandler *currHandler = dynamic_cast<DynamicHandler*>(handler);

	if (currHandler != NULL)
		currHandler->SendReqtoCgi(client);
	else
	{
		StaticHandler *currHandler = dynamic_cast<StaticHandler*>(handler);
		currHandler->sendReqtoEvent(client);
	}
}

void HttpRequestManager::DynamicRunCgi(Client& client)
{
	DynamicHandler *currHandler = dynamic_cast<DynamicHandler*>(handler);

	if (currHandler != NULL)
		currHandler->RunCgi(client);
}

void HttpRequestManager::DynamicMakeResponse(Client& client)
{
	DynamicHandler *currHandler = dynamic_cast<DynamicHandler*>(handler);

	if (currHandler != NULL)
		currHandler->MakeResponse(client);
}

void HttpRequestManager::DynamicReadFromCgi(Client& client)
{
	DynamicHandler *currHandler = dynamic_cast<DynamicHandler*>(handler);

	if (currHandler != NULL)
		currHandler->ReadFromCgi(client);
}
