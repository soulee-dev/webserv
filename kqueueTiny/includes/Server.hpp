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
# include <fcntl.h>
# include <sys/event.h>
# include <sys/time.h>
# include <sys/types.h>
# include <unistd.h>

# include "request.hpp"

# define BUFFER_SIZE 30720
# define SERVER_NAME "Tiny Web Server"
# define CRLF "\r\n"

class Server
{
	private:
		const std::string		server_ip_addr;
		const unsigned short	server_port;
		unsigned int			server_socket;
		long					server_incoming_message;
		struct sockaddr_in		server_sockaddr;
		unsigned int			server_sockaddrlen;
		int	kq;
		
		std::map<std::string, std::map<std::string, std::string> >	sessions;

		// REMOVE BEFORE FLIGHT
		std::map<std::string, std::string>	_config;
		std::vector<std::string>			index;
		std::map<std::string, std::string>	cookies;
		// REMOVE BEFORE FLIGHT

		// KQUEUE METHODS
		std::map<int, std::string> clients;
		std::vector<struct kevent> change_list;
		struct kevent event_list[8];

		std::string	BuildHeader(std::string status_code, int file_size, std::string file_type, std::map<std::string, std::string> cookies);
		std::string BuildHeader(std::string status_code, int file_size, std::string file_type);
		void		ServeStatic(Request& req);
		void		ServeDynamic(Request& req);
		void		ServeAutoIndex(Request& req);
		void		ClientError(int fd, std::string cause, std::string error_num, std::string short_msg, std::string long_msg);
		void		ProcessTraffic(struct kevent* curr_event, std::vector<struct kevent> &change_list, int clientfd, int serverfd);
		void		ParseURI(std::string uri, Request &req);
		void		disconnectFd(int client_fd, std::map<int, std::string>& clients);

	public:
		Server(std::string ip_addr, unsigned int port);
		~Server();
		void		run();
		void		nb_run();
		int getkq();
};
