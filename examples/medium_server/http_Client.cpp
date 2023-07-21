/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   http_Client.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jinam <jinam@student.42seoul.kr>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/20 15:19:15 by jinam             #+#    #+#             */
/*   Updated: 2023/07/20 16:16:55 by jinam            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "http_Client.hpp"
#include <cstring>
#include <unistd.h>
Client::Client()
{
    memset(m_buf, 0, BUFFERSIZE);
}

Client::~Client()
{
    close(m_new_socket);
}
Client::Client(const Client& ref)
{
}
Client& Client::operator=(const Client& ref)
{
    return *this;
}

int& Client::getClientSocket()
{
    return m_new_socket;
}

int Client::readRequest(void)
{
    int bytesReceived = read(m_new_socket, m_buf, BUFFERSIZE);

    if (bytesReceived < 0)
        return -1;
    return 0;
}

std::string Client::makingResponseMessage()
{
    if (m_req.getMethod() == http::GET)
    {
        std::string htmlFile = "<!DOCTYPE html><html lang=\"en\"><body><h1> HOME </h1><p> TEST \nTEST\n Hello from your Server :) </p></body></html>";
        std::ostringstream ss;
        ss << "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: " << htmlFile.size() << "\n\n"
           << htmlFile;

        return ss.str();
    }
    return ("TEST");
}
