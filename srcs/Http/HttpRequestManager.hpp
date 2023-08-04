#pragma once

#include "../Client.hpp"
#include "HttpParser.hpp"
#include "Handler/Handler.hpp"
#include "Handler/StaticHandler.hpp"
#include "Handler/DynamicHandler.hpp"

class HttpRequestManager
{
	private:
		HttpParser	parser;
		Handler*	handler;
	public:
		HttpRequestManager(Client& client);
		void	processRequest(void);
		~HttpRequestManager();
};
