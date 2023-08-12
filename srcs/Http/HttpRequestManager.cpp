#include "HttpRequestManager.hpp"

HttpRequestManager::HttpRequestManager()
{

}

void	HttpRequestManager::setHandler(std::vector<std::string> List)
{
	getFrontReq() = parse(List);

	if (getFrontReq().is_static)
	{
		std::cout << BOLDRED << " -- PROCESSING STATIC -- \n";
		handler = new StaticHandler();
		handler->handlerType = STATIC;
	}
	else
	{
		std::cout << BOLDBLUE << " -- PROCESSING DYNAMIC --\n";
		handler = new DynamicHandler();
		handler->handlerType = DYNAMIC;
	}
}

HttpRequest	HttpRequestManager::parse(std::vector<std::string> List)
{
	HttpRequest		result;
	HttpRequest		request = getFrontReq();
	
	std::istringstream	iss(List.back());
	List.pop_back();
	iss >> result.isAutoIndex;
	result.root = List.back();
	List.pop_back();
	result.method = request.method;
	std::cout << BOLDMAGENTA << "METHOD : " << result.method << '\n';
	
	result.indexList = List;
	result.errnum = 0;

	if (request.uri.find("cgi-bin") == std::string::npos)
	{
		// When static
		result.is_static = true;
		result.path = request.uri;
		std::cout << BOLDYELLOW << "URI(PATH) : " << request.uri << '\n';
		result.file_name = result.root; // + result.path;
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
	// delete handler;
}

void	HttpRequestManager::pushReq()
{
	queReq.push(HttpRequest());
}

int		HttpRequestManager::getHandlerType()
{
	return handler->handlerType;
}

void HttpRequestManager::dynamicOpenFd(Client& client)
{
	DynamicHandler *currHandler = dynamic_cast<DynamicHandler*>(handler);

	if (currHandler != NULL)
		currHandler->openFd(client);
}
void HttpRequestManager::dynamicSendReqtoCgi(Client& client)
{
	DynamicHandler *currHandler = dynamic_cast<DynamicHandler*>(handler);

	if (currHandler != NULL)
		currHandler->sendReqtoCgi(client);
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