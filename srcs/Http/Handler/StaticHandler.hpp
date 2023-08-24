#pragma once

#include "Handler.hpp"

class StaticHandler
{
	public:
		std::vector<unsigned char>	handle(Client& client) const;
		~StaticHandler();
		void sendReqtoEvent(Client& client);
		std::vector<unsigned char>	ProcessDirectory(Client& client) const;
		std::vector<unsigned char>	HandleDirectoryListing(Client& client, Request& request) const;
};

int	is_directory(std::string fileName);
