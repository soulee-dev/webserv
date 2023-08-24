#pragma once

#include "Handler.hpp"

class ErrorHandler
{
	private:

	public:
		static std::vector<unsigned char>	handle(Client &client, int status_code);
		static void sendReqtoError(Client &client);
};
