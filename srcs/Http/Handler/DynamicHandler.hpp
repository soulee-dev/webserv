#pragma once

#include "Handler.hpp"

class DynamicHandler : public Handler
{
	public:
		std::vector<unsigned char>	handle(HttpRequest& request);
};
