#pragma once

#include "Handler.hpp"

class ErrorHandler : public Handler
{
	public:
		std::vector<unsigned char>	handle(Client& client) const;
		~ErrorHandler();
};

void	noDirectory(HttpRequest& request);
void	noRegUsr(HttpRequest& request);
void	noAutoIndex(HttpRequest& request);
std::string	build_header_err(std::string status_code, int file_size, std::string file_type);
