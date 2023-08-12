#pragma once

#include "Handler.hpp"

class DirectoryIndexHandler : public Handler
{
	public:
		std::vector<unsigned char>	handle(Client& client);
		void openfd(Client& client);
};
