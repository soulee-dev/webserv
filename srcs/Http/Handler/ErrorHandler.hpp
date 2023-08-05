#pragma once

#include "Handler.hpp"

class ErrorHandler : public Handler
{
	public:
		std::vector<unsigned char>	handle(HttpRequest& request) const;
};
