#pragma once

#include "Handler.hpp"

class ErrorHandler : public Handler
{
	private:

	public:
		static std::vector<unsigned char>	handler(int status_code);
};
