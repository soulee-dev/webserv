#pragma once

#include "Handler.hpp"

class StaticHandler : public Handler
{
	public:
		std::vector<unsigned char>	handle(Client& client) const;
		~StaticHandler();
		void sendReqtoEvent(Client& client);
};

std::string	getFileType(std::string file_name);
std::string	build_header(std::string status_code, int file_size, std::string file_type);
void	processDirectory(Client& client);
void	MakeStaticResponse(HttpRequest& request);
int	is_directory(std::string fileName);
