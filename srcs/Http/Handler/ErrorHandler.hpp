#pragma once

#include "Handler.hpp"

class ErrorHandler : public Handler
{
	private:

	public:
		static std::vector<unsigned char>	handle(Client &client, int status_code);
		std::vector<unsigned char>	handle(Client& client);
		static void 				sendReqtoError(Client &client);
};

std::string	find_error_page(int status_code); // 에러 페이지 반환