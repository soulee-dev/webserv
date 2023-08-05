#pragma once

#include "Handler.hpp"

class StaticHandler : public Handler
{
	public:
		std::vector<unsigned char>	handle(HttpRequest& request);
};
