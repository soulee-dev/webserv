#pragma once
#include <map>
#include <string>
#include <vector>

# define CRLF "\r\n"
class Message
{
    public:
        int         status_code;
        std::string	method;
        std::string	http_version;
        std::map<std::string, std::string> headers;
        std::vector<unsigned char> body;

        void    clear(void);
};
