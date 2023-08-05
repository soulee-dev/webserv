#include "HttpRequestManager.hpp"

HttpRequestManager::HttpRequestManager(Client& client)
{
	request = parser.parse(client);
	
	// if Static
	if (request.is_static)
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

std::vector<unsigned char>	HttpRequestManager::processRequest(void)
{
	return handler->handle(request);
}

HttpRequestManager::~HttpRequestManager()
{
	delete handler;
}
