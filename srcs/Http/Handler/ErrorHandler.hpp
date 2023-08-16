#pragma once

#include "Handler.hpp"

class ErrorHandler : public Handler
{
	private:

	public:
		static std::vector<unsigned char>	handle(Client &client, int status_code);
		std::vector<unsigned char>	handle(Client& client);
		static void sendReqtoError(Client &client); // Handler 외부에서 Errorhandler 호출 후, sendbuffer에 전송합니다.
};
