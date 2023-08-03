#pragma once

#include <string>
#include <iostream>
#include "Client.hpp"

class Response
{
	private:
		bool			is_static;
		bool			is_directory;

	public:
		Response(Client& client);
		virtual void build() = 0;
		virtual	~Response();
};
