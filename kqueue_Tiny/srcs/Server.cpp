/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gyopark <gyopark@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/23 12:21:28 by jinam             #+#    #+#             */
/*   Updated: 2023/07/28 16:23:11 by gyopark          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"

Server::~Server()
{
	close(server_socket);
	close(kq);
	// close(_new_socket);
}

Server::Server(std::string ip_addr, unsigned int port)
	: server_ip_addr(ip_addr), server_port(port), server_sockaddrlen(sizeof(server_sockaddr))
{
	int opt = 1;

	server_sockaddr.sin_family = AF_INET;
	server_sockaddr.sin_port = htons(server_port);
	server_sockaddr.sin_addr.s_addr = inet_addr(server_ip_addr.c_str());
	if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		throw std::runtime_error("Cannot create socket");
	setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	fcntl(server_socket, F_SETFL, O_NONBLOCK);
	if (bind(server_socket, (sockaddr *)&server_sockaddr, server_sockaddrlen) < 0)
		throw std::runtime_error("Cannot bind socket to address");

	cookies["foo"] = "bar";
	_config["root"] = "/html";
	index.push_back("index.html");
	index.push_back("index.htm");
	_config["autoindex"] = "on";

	this->kq = kqueue();
	if (this->kq == -1)
	{
		std::cout << "kqueue error" << std::endl;
		exit(1);
	}
}
