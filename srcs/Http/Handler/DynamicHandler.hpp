#pragma once

#include "Handler.hpp"
#include <unistd.h>

class DynamicHandler : public Handler
{
	public:
		std::vector<unsigned char>	handle(Client& client) const;
		bool OpenFd(Client& client);
		void SendReqtoCgi(Client& client);
		void RunCgi(Client& client);
		void MakeResponse(Client& client);
		void ReadFromCgi(Client& client);
		~DynamicHandler();
};
