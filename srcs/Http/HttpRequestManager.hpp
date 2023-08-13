#pragma once

#include <vector>
#include <queue>
#include "Handler/Handler.hpp"
#include "Handler/StaticHandler.hpp"
#include "Handler/DynamicHandler.hpp"
#include "Handler/ErrorHandler.hpp"
#include "Handler/DeleteHandler.hpp"

class Client;

class HttpRequestManager
{
	private:
		std::queue<HttpRequest> queReq;  // 가져갈땐 pop, 넣을땐 push
		Handler*	handler;
	public:
		HttpRequestManager(); 
		HttpRequest	parse(Client &client, std::string foundUri, std::string foundFile);
		void	setHandler(Client &client, std::string foundUri, std::string foundFile);
		std::vector<unsigned char>	processRequest(Client& client);
		~HttpRequestManager();
		HttpRequest& getRequest();

		void			pushReq(void);
		HttpRequest& 	getBackReq(void);
		HttpRequest&	getFrontReq(void);
		HttpRequest		popReq(void);

		void dynamicOpenFd(Client& client); // dynamic
		void sendReqtoEvent(Client& client); //for all(static and dynamic)
		void dynamicRunCgi(Client& client); //dynamic
		void dynamicMakeResponse(Client& client);
		void dynamicReadFromCgi(Client& client);
};
