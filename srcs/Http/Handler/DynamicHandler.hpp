#pragma once

#include "Handler.hpp"
#include <unistd.h>

class DynamicHandler
{
	public:
		void OpenFd(Client& client);
		void RunCgi(Client& client);
		void MakeResponse(Client& client);
};
