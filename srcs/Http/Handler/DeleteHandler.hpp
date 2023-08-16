#pragma once

#include "Handler.hpp"

class DeleteHandler : public Handler
{
	public:
		std::vector<unsigned char>	handle(Client &client) const;
		void sendReqtoDelete(Client &client);
};
