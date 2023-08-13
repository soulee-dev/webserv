#pragma once

#include <vector>
#include <queue>
#include "Handler/Handler.hpp"
#include "Handler/StaticHandler.hpp"
#include "Handler/DynamicHandler.hpp"
#include "Handler/ErrorHandler.hpp"

class Client;

class HttpRequestManager
{
	private:
		std::queue<HttpRequest> queReq;  // 가져갈땐 pop, 넣을땐 push
		Handler*	handler;
	public:
		HttpRequestManager(); 
		HttpRequest	parse(std::vector<std::string> List);
		void	setHandler(std::vector<std::string> List);
		std::vector<unsigned char>	processRequest(Client& client);
		~HttpRequestManager();
		HttpRequest& getRequest();

		void			pushReq(void);
		HttpRequest& 	getBackReq(void);
		HttpRequest&	getFrontReq(void);
		HttpRequest		popReq(void);

		void dynamicOpenFd(Client& client);
		void dynamicSendReqtoCgi(Client& client);
		void dynamicRunCgi(Client& client);
		void dynamicMakeResponse(Client& client);
		void dynamicReadFromCgi(Client& client);
};
