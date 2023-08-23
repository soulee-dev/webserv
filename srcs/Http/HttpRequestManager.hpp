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
		HttpRequest& getRequest();

		void DynamicOpenFd(Client& client); // dynamic
		void SendReqtoEvent(Client& client); //for all(static and dynamic)
		void DynamicRunCgi(Client& client); //dynamic
		void DynamicMakeResponse(Client& client);
		void DynamicReadFromCgi(Client& client);
};
