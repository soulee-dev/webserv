#include "HttpRequestManager.hpp"

HttpRequestManager::HttpRequestManager(Client& client)
{
	parser.parse(client);
	
	// if Static
	if (1)
	{
		handler = new StaticHandler();
	}
	else
	{
		handler = new DynamicHandler();
	}
}

void	HttpRequestManager::processRequest(void)
{
	handler->handle();
}

HttpRequestManager::~HttpRequestManager()
{
	delete handler;
}
