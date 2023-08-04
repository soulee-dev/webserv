#include "HttpRequestManager.hpp"

HttpRequestManager::HttpRequestManager(Client& client)
{
	request = parser.parse(client);
	
	// if Static
	if (request.is_static)
		handler = new StaticHandler();
	else
		handler = new DynamicHandler();
}

std::vector<unsigned char>	HttpRequestManager::processRequest(void)
{
	return handler->handle(request);
}

HttpRequestManager::~HttpRequestManager()
{
	delete handler;
}
