#pragma once

#include "Handler.hpp"

class StaticHandler : public Handler
{
	public:
		std::vector<unsigned char>	handle(Client& client) const;
		~StaticHandler();
		void sendReqtoEvent(Client& client);
		std::vector<unsigned char>	ProcessDirectory(Client& client) const;
		std::vector<unsigned char>	HandleDirectoryListing(std::string& path) const;
};

int	is_directory(std::string fileName);
