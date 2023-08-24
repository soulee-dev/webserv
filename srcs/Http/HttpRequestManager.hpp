#pragma once

#include <vector>

#include "Handler/Handler.hpp"
#include "Handler/StaticHandler.hpp"
#include "Handler/DynamicHandler.hpp"
#include "Handler/ErrorHandler.hpp"
#include "Handler/DeleteHandler.hpp"

class Client;

class HttpRequestManager
{
	private:
		Handler*	handler;
	public:
		HttpRequestManager();
		void		Parse(Client& client);
		void		SetHandler(Client& client);
		std::vector<unsigned char>	processRequest(Client& client);
		~HttpRequestManager();

		void DynamicOpenFd(Client& client);
		void SendReqtoEvent(Client& client);
		void DynamicRunCgi(Client& client);
		void DynamicMakeResponse(Client& client);
		void DynamicReadFromCgi(Client& client);
};
