#pragma once

#include "Handler.hpp"
#include <unistd.h>

class DynamicHandler : public Handler
{
	public:
		std::vector<unsigned char>	handle(HttpRequest& request);
};
