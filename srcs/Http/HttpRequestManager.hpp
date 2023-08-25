#pragma once

#include <vector>

#include "Handler/Handler.hpp"
#include "Handler/StaticHandler.hpp"
#include "Handler/ErrorHandler.hpp"
#include "Handler/DeleteHandler.hpp"
#include "Handler/DynamicHandler.hpp"

class Client;

class HttpRequestManager
{
	public:
		void	Parse(Client& client);
		void	Handle(Client& client);
};
