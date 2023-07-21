/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   http_tcpServer.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jinam <jinam@student.42seoul.kr>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/20 14:42:22 by jinam             #+#    #+#             */
/*   Updated: 2023/07/20 16:25:30 by jinam            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "http_tcpServer.hpp"
#include "http_Client.hpp"
#include <iostream>
#include <sstream>
#include <unistd.h>

namespace
{
    void log(const std::string& message)
    {
        std::cout << message << std::endl;
    }
    void exitWithError(const std::string& errorMessage)
    {
        log("ERROR : " + errorMessage);
        exit(1);
    }
}

http::TcpServer::TcpServer()
{
}

http::TcpServer::TcpServer(std::string ip_address, int port)
{
    m_server.m_ip_address = ip_address;
    m_server.m_port = port;
    m_server.m_socket = 0;

    m_server.m_socketAddress_len = sizeof(m_server.m_socketAddress);
    m_server.m_socketAddress.sin_family = AF_INET;
    m_server.m_socketAddress.sin_port = htons(m_server.m_port);
    m_server.m_socketAddress.sin_addr.s_addr = inet_addr(m_server.m_ip_address.c_str());

    if (startServer() != 0)
    {
        std::ostringstream ss;
        ss << "Failted to start server with PORT : " << ntohs(m_server.m_socketAddress.sin_port);
        log(ss.str());
    }
}
http::TcpServer::~TcpServer()
{
    closeServer();
}
http::TcpServer::TcpServer(const TcpServer& ref)
{
}
http::TcpServer& http::TcpServer::operator=(const TcpServer& ref)
{
    return *this;
}

int http::TcpServer::startServer(void)
{
    int& _socket = m_server.m_socket;
    sockaddr* _socketAddress = (sockaddr*)&m_server.m_socketAddress;

    _socket = socket(AF_INET, SOCK_STREAM, 0);
    if (_socket < 0)
    {
        exitWithError("Cannot create socket");
        return 1;
    }
    if (bind(_socket, _socketAddress, m_server.m_socketAddress_len) < 0)
    {
        exitWithError("Cannot connect socket to address");
        return 1;
    }
    return 0;
}

void http::TcpServer::closeServer(void)
{
    close(m_server.m_socket);
    exit(0);
}

void http::TcpServer::startListen(void)
{
    int& _socket = m_server.m_socket;
    int _CAP = 20;

    if (listen(_socket, _CAP) < 0)
    {
        exitWithError("Socket listen failed");
    }
    std::ostringstream ss;
    ss << "\n ** listen on ADDRESS : " << inet_ntoa(m_server.m_socketAddress.sin_addr)
       << " PORT : " << ntohs(m_server.m_socketAddress.sin_port)
       << "** \n";
    log(ss.str());

    while (true)
    {
        log("=== Waiting for a new connection ===\n\n\n");
        Client _newClient;

        acceptConnection(_newClient.getClientSocket());
        if (_newClient.readRequest() < 0)
        {
            exitWithError("Failed to read bytes from client socket connection");
        }
        std::ostringstream ss;
        ss << "--- Received Request from client ---\n\n";
        log(ss.str());
        sendResponse(_newClient.getClientSocket(), buildResponse(_newClient));
    }
}

void http::TcpServer::acceptConnection(int& new_socket)
{
    int& _socket = m_server.m_socket;
    sockaddr* _socketAddress = (sockaddr*)&m_server.m_socketAddress;

    new_socket = accept(_socket, _socketAddress, &m_server.m_socketAddress_len);
    if (new_socket < 0)
    {
        std::ostringstream ss;
        ss << "Server failed to accept incoming connection from ADDRESS: "
           << inet_ntoa(m_server.m_socketAddress.sin_addr)
           << "; PORT : "
           << ntohs(m_server.m_socketAddress.sin_port);
        exitWithError(ss.str());
    }
}

std::string http::TcpServer::buildResponse(Client& cli)
{
    return cli.makingResponseMessage();
}

void http::TcpServer::sendResponse(int& _new_socket, std::string msg)
{
    long bytesSent;
    std::string res;

    bytesSent = write(_new_socket, msg.c_str(), msg.size());
    if (bytesSent == msg.size())
        res = "---Server Response sent to client ---\n\n";
    else
        res = "Error sending response to client";
    log(res);
}
