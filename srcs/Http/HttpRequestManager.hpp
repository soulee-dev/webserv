#pragma once

#include <vector>
#include <deque>

#include "Handler/Handler.hpp"
#include "Handler/StaticHandler.hpp"
#include "Handler/DynamicHandler.hpp"
#include "Handler/ErrorHandler.hpp"
#include "Handler/DeleteHandler.hpp"

class Client;

class HttpRequestManager
{
	private:
		std::deque<HttpRequest> queReq;  // 가져갈땐 pop, 넣을땐 push
		Handler*	handler;
	public:
		HttpRequestManager(); 
		void		Parse(Client& client);
		void		SetHandler(Client& client);
		std::vector<unsigned char>	processRequest(Client& client);
		~HttpRequestManager();
		HttpRequest& getRequest();

		void			pushReq(void);
		HttpRequest& 	getBackReq(void);
		HttpRequest&	getFrontReq(void);
		void			popFrontReq(void);
		void			popBackReq(void);

		void DynamicOpenFd(Client& client); // dynamic
		void SendReqtoEvent(Client& client); //for all(static and dynamic)
		void DynamicRunCgi(Client& client); //dynamic
		void DynamicMakeResponse(Client& client);
		void DynamicReadFromCgi(Client& client);
};
