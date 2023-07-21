/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   http_tcpServer.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jinam <jinam@student.42seoul.kr>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/20 14:03:04 by jinam             #+#    #+#             */
/*   Updated: 2023/07/20 16:22:12 by jinam            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_TCPSERVER_HPP
#define HTTP_TCPSERVER_HPP
#include "http_Client.hpp"
#include <arpa/inet.h>
#include <string>
#include <sys/socket.h>
namespace http
{
    class Request;
    class TcpServer
    {
        struct server
        {
            std::string m_ip_address;
            int m_port;
            int m_socket;
            // int m_new_socket;
            struct sockaddr_in m_socketAddress;
            unsigned int m_socketAddress_len;
        };

    private:
        server m_server;

    private:
        int startServer();
        void closeServer();
        void acceptConnection(int& new_socket);
        std::string buildResponse(Client& cli);
        void sendResponse(int& new_socket, std::string msg);

    public:
        TcpServer();
        TcpServer(std::string ip_address, int port);
        ~TcpServer();
        TcpServer(const TcpServer& ref);
        TcpServer& operator=(const TcpServer& ref);
        void startListen(void);
    };
}

#endif
