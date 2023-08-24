#pragma once

#include "Handler.hpp"

class ErrorHandler : public Handler
{
	private:

	public:
		static std::vector<unsigned char>	handle(Client &client, int status_code);
		std::vector<unsigned char>	handle(Client& client);
		static void sendReqtoError(Client &client);
};
