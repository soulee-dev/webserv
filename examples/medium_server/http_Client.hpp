/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   http_Client.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jinam <jinam@student.42seoul.kr>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/20 15:00:32 by jinam             #+#    #+#             */
/*   Updated: 2023/07/20 16:15:02 by jinam            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_CLIENT_HPP
#define HTTP_CLIENT_HPP
#include "http_Request.hpp"
#define BUFFERSIZE 30720
class Client
{
private:
    int m_new_socket;
    char m_buf[BUFFERSIZE];
    http::Request m_req;
    std::string m_response;

public:
    Client();
    ~Client();
    Client(const Client& ref);
    Client& operator=(const Client& ref);
    int& getClientSocket(void);
    int readRequest(void);
    int parsingRequest(void);
    int processingRequest(void);
    std::string makingResponseMessage(void);
};

#endif
