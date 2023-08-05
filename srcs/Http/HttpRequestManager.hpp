#pragma once

#include <vector>
#include "../Client.hpp"
#include "HttpParser.hpp"
#include "Handler/Handler.hpp"
#include "Handler/StaticHandler.hpp"
#include "Handler/DynamicHandler.hpp"

class HttpRequestManager
{
	private:
		HttpRequest	request;
		HttpParser	parser;
		Handler*	handler;
	public:
		HttpRequestManager(Client& client);
		std::vector<unsigned char>	processRequest(void);
		~HttpRequestManager();
};
