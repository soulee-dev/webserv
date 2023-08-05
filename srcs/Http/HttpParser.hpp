#pragma once

#include "../Client.hpp"
#include "HttpRequest.hpp"
#include "../Color.hpp"
#include <iostream>

class HttpParser
{
	public:
		static HttpRequest	parse(Client& client);
};