#pragma once

#include <vector>
#include "../Client.hpp"
#include "HttpParser.hpp"
#include "Handler/Handler.hpp"
#include "Handler/StaticHandler.hpp"
#include "Handler/DynamicHandler.hpp"
#include "Handler/ErrorHandler.hpp"

class HttpRequestManager
{
	private:
		HttpRequest	request;
		HttpParser	parser;
		Handler*	handler;
	public:
		HttpRequestManager(Client& client, std::vector<std::string> List, int block);
		std::vector<unsigned char>	processRequest(void);
		~HttpRequestManager();
		HttpRequest	getRequest() const;
};
