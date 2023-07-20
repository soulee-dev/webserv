#include <iostream>
#include <sstream>
#include <string>
#include "../tiny_webserv/csapp.h"
#include "color.hpp"

#define BUFFER_SIZE 30720

class Server
{
	private:
		std::string _ip;
		int	_port;
		int	_socket;
		int	_new_socket;
		struct sockaddr_in sock_addr;
		unsigned int sock_addrlen;

		std::string _serverMessage;

	public:
		Server(std::string ip, int port);
		~Server();
		void Listen();

		int launch_server();
		void close_server();
		void accept_connection(int &new_socket);
		void send_response();
		std::string	build_response();
		// void	run(std::string ip, std::string port);
};
