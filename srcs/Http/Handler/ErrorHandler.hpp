#pragma once

#include "Handler.hpp"

class ErrrorHandler : public Handler
{
	public:
		std::vector<unsigned char>	handle(HttpRequest& request) const;
};