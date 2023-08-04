#pragma once

#include "../Client.hpp"
#include "HttpRequest.hpp"

class HttpParser
{
	public:
		static HttpRequest	parse(Client& client);
};