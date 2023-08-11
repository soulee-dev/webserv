#pragma once

#include "Handler.hpp"
#include <unistd.h>

class DynamicHandler : public Handler
{
	public:
		std::vector<unsigned char>	handle(Client& client) const;
		~DynamicHandler();
};

std::string	getFileTypeD(std::string file_name);
