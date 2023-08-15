#pragma once

#include "Handler.hpp"

class ErrorHandler : public Handler
{
	private:

	public:
		static std::vector<unsigned char>	handler(int status_code);
		static void sendReqtoError(Client &client); // Handler 외부에서 Errorhandler 호출 후, sendbuffer에 전송합니다.
};

std::string	find_error_page(int status_code); // 에러 페이지 반환