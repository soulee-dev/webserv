#pragma once

#include "Handler.hpp"

class AutoIndexHandler : public Handler
{
	public:
		std::vector<unsigned char>	handle(HttpRequest& request) const;
};