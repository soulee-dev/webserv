#pragma once

#include "Handler.hpp"

class ErrorHandler : public Handler
{
	private:

	public:
		ErrorHandler(Client const&);
		static std::vector<unsigned char>	handler(Client& client, int status_code);
};
