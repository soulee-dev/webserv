/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   http_Request.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jinam <jinam@student.42seoul.kr>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/20 14:46:28 by jinam             #+#    #+#             */
/*   Updated: 2023/07/20 16:16:07 by jinam            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP
#include <map>
#include <sstream>
#include <string>
namespace http
{
    enum method
    {
        GET,
        PUT,
        DELETE,
        CGI,
        ERR
    };
    enum parsingstatus
    {
        READY,
        COMPLETE,
        ERROR
    };

    class Request
    {
    private:
        std::stringstream m_raw;
        int m_parsingStatus;

        int m_method;
        std::string m_path;
        std::map<std::string, std::string> m_headers;
        std::string m_body;

    public:
        Request();
        Request(char* msg);
        ~Request();
        Request(const Request& ref);
        Request& operator=(const Request& ref);
        int parsingStartline(void);
        int parsingHeaders(void);
        int parsingBody(void);
        int getMethod(void);
    };
}

#endif
