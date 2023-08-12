#pragma once

#include "Handler.hpp"
#include <unistd.h>

class DynamicHandler : public Handler
{
	public:
		std::vector<unsigned char>	handle(Client& client) const;
		void openFd(Client& client);
		void sendReqtoCgi(Client& client);
		void runCgi(Client& client);
		void makeResponse(Client& client);
		void readFromCgi(Client& client);
		~DynamicHandler();
};

std::string	getFileTypeD(std::string file_name);
