#pragma once

#include "Handler.hpp"

class ErrrorHandler : public Handler
{
	public:
		
};


// if (ret_stat < 0)
// {
// 	std::cout << "404 ";
// 	ClientError(req.fd, req.file_name, "404", "Not found", "Tiny couldn't find this file");
// 	return ;
// }

// void	Server::ClientError(int fd, std::string cause, std::string error_num, std::string short_msg, std::string long_msg)
// {
// 	std::string htmlFile = "<html><title>Tiny Error</title><body bgcolor=""ffffff"">" + error_num + ":" + short_msg + "<p>" + long_msg + ":" + cause + "</p> <hr><em>The Tiny Web server</em></body></html>";
//     std::string	message = BuildHeader(error_num + " " + short_msg, htmlFile.size(), "text/html") + htmlFile;

// 	ssize_t	sent_bytes;

// 	if ((sent_bytes = send(fd, message.c_str(), message.size(), 0)) < 0)
// 		throw std::runtime_error("Send failed");
// 	if (sent_bytes == message.size())
// 		std::cout << "Successfully send message" << std::endl;
// 	else
// 		std::cout << "Error" << std::endl;
// }