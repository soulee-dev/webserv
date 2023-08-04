#include "StaticHandler.hpp"

std::vector<unsigned char>	StaticHandler::handle(HttpRequest& request) const
{
	std::string	s = "HTTP/1.1 404 Not Found\r\nServer: nginx/1.25.1\r\nDate: Fri, 28 Jul 2023 12:42:57 GMT\r\n\
                    Content-Type: text/html\r\nContent-Length: 153\r\nConnection: keep-alive\r\n\r\n<html>\r\n\
                    <head><title>404 Not Found</title></head>\r\n<body>\r\n<center><h1>Hello my name is jj!!</h1></center>\r\n\
                    <hr><center>webserv 0.1</center>\r\n</body>\r\n</html>";
	
	std::vector<unsigned char>	result(s.begin(), s.end());
	return (result);
}
