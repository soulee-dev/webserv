#pragma once

# include <stdio.h>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <sys/stat.h>
# include <stdlib.h>
# include <string>
# include <iostream>
# include <fstream>
# include <cstdio>
# include <sstream>
# include <unistd.h>
# include <dirent.h>
# include <vector>
# include <map>

# include "request.hpp"

# define BUFFER_SIZE 30720
# define SERVER_NAME "Tiny Web Server"
# define CRLF "\r\n"

class Server
{
	private:
		const std::string		_ip_addr;
		const unsigned short	_port;
		unsigned int			_socket;
		long					_incoming_message;
		struct sockaddr_in		_sock_addr;
		unsigned int			_sock_addr_len;
		
		std::map<std::string, std::map<std::string, std::string> >	sessions;

		// REMOVE BEFORE FLIGHT
		std::map<std::string, std::string>	_config;
		std::vector<std::string>			index;
		std::map<std::string, std::string>	cookies;
		// REMOVE BEFORE FLIGHT

		std::string	BuildHeader(std::string status_code, int file_size, std::string file_type, std::map<std::string, std::string> cookies);
		std::string BuildHeader(std::string status_code, int file_size, std::string file_type);
		void		ServeStatic(Request& req);
		void		ServeDynamic(Request& req);
		void		ServeAutoIndex(Request& req);
		void		ClientError(int fd, std::string cause, std::string error_num, std::string short_msg, std::string long_msg);
		void		ProcessTraffic(int fd);
		void		ParseURI(std::string uri, Request &req);

public:
	Server(std::string ip_addr, unsigned int port);
	~Server();
	void		run();
	void		nb_run();
};
