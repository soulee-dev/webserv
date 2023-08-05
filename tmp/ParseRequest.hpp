#ifndef PARSEREQUEST_HPP
# define PARSEREQUEST_HPP

# include "Client.hpp"
# include "RequestMessage.hpp"
# include "ResponseMessage.hpp"
# include <iostream>
# include <string>
# include <fstream>
# include <sstream>

#include <sys/stat.h> // for struct stat
#include <dirent.h> // for directory listing

#define CRLF "\r\n"
#define STATIC 1001
#define DYNAMIC 1002

class ParseRequest
{
    private:
        ParseRequest();
        
    public:
        ~ParseRequest();
        static void parseRequest(Client &c, ResponseMessage &r);
};

std::string getFileType(std::string FileName);
std::string build_header(std::string status_code, int file_size, std::string file_type);
void    processStatic(Client &c, ResponseMessage &r, std::string uri); 
void    processDirectory(Client &c);
void    MakeStaticResponse(Client &c, ResponseMessage &r);

#endif
