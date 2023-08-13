#include "HttpRequestManager.hpp"
#include "../Client.hpp"

HttpRequestManager::HttpRequestManager() {}

void	HttpRequestManager::setHandler(Client &client, std::string foundUri, std::string foundFile)
{
	getFrontReq() = parse(client, foundUri, foundFile);

	if (getFrontReq().is_static)
	{
		if (getFrontReq().method == "DELETE") // process Method DELETE
		{
			std::cout << "DELETE네?\n";
			handler = new DeleteHandler();
			return ;
		}
		std::cout << BOLDRED << " -- PROCESSING STATIC -- \n";
		handler = new StaticHandler();
	}
	else
	{
		std::cout << BOLDBLUE << " -- PROCESSING DYNAMIC --\n";
		handler = new DynamicHandler();
	}
}

HttpRequest	HttpRequestManager::parse(Client &client, std::string foundUri, std::string foundFile)
{
	HttpRequest		result;
	HttpRequest		request = getFrontReq();
	
	result.isAutoIndex = client.getServer()->getLocations()[foundUri].getAutoIndex();
	result.root = client.getServer()->getLocations()[foundUri].getRoot();
	std::cout << BOLDBLUE << "ROOT : " << result.root << '\n';
	result.method = request.method;
	std::cout << BOLDMAGENTA << "METHOD : " << result.method << '\n';
	result.errnum = 0;

	std::string	fileName;
	if (foundFile.empty())
		fileName = client.getServer()->getLocations()[foundUri].getIndex()[0];
	else
		fileName = foundFile;

	if (request.uri.find("cgi-bin") == std::string::npos)
	{
		// When static
		result.is_static = true;
		result.path = request.uri;
		std::cout << BOLDYELLOW << "URI(PATH) : " << request.uri << '\n';
		result.file_name = result.root + "/" + fileName; // + result.path;
		std::cout << BOLDGREEN << "FILE NAME : " << result.file_name << '\n';
	}
	else
	{
		result.is_static = false;
		size_t	location = request.uri.find('?');
		if (location != std::string::npos)
			result.cgi_args = request.uri.substr(location + 1);
		result.file_name = "." + request.uri.substr(0, location);
		result.body = request.body;
	}
	return result;
};

HttpRequest& HttpRequestManager::getBackReq(void) { return queReq.back(); }

HttpRequest& HttpRequestManager::getFrontReq(void) { return queReq.front(); }

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
	// TODO: Segfault when destructing instance
	// delete handler;
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
	else if (getFrontReq().method == "DELETE")
	{
		DeleteHandler *currHandler = dynamic_cast<DeleteHandler*>(handler);
		currHandler->sendReqtoDelete(client);
	}
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
