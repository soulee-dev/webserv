#pragma once

#include "../HttpRequest.hpp"
#include <vector>

class Handler
{
	public:
		virtual std::vector<unsigned char>	handle(HttpRequest& request) const = 0;
};