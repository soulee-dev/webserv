#include "ErrorHandler.hpp"
#include "../../Client.hpp"

std::vector<unsigned char> ErrorHandler::handle(Client& client) const 
{
	HttpRequest	request = client.httpRequestManager.getRequest();

	int	_errnum = request.errnum;
	std::vector<unsigned char> _ubuffer;
	std::cout << BOLDRED << "ERRNUM : " << _errnum << '\n';

	switch (_errnum)
	{
		case 1:
			noDirectory(request);
			break;
		case 2:
			noRegUsr(request);
			break;
		case 3:
			noAutoIndex(request);
			break;
	}
	_ubuffer.insert(_ubuffer.end(), request.header.begin(), request.header.end());
	_ubuffer.insert(_ubuffer.end(), request.ubuffer.begin(), request.ubuffer.end());
	return _ubuffer;
}

void	noDirectory(HttpRequest& request)
{
	std::string	header = "<h1>404 File Not Found</h1>";
	std::string buffer = "<h1>404 File Not Found</h1>";

	request.header = build_header_err("404 File Not Found", header.length(), "text/html");
    request.ubuffer.insert(request.ubuffer.end(), buffer.begin(), buffer.end());
}

void	noRegUsr(HttpRequest& request)
{
	std::string	header = "<h1>401 Unauthorized</h1>";
	std::string buffer = "<h1>401 Unauthorized</h1>";
	std::vector<unsigned char> ubuffer;

	request.header = build_header_err("404 File Not Found", header.length(), "text/html");
    request.ubuffer.insert(request.ubuffer.end(), buffer.begin(), buffer.end());
}

void	noAutoIndex(HttpRequest& request)
{
	std::string	header = "<h1>404 File Not Found</h1>";
	std::string buffer = "<h1>404 File Not Found</h1>";
	std::vector<unsigned char> ubuffer;

	request.header = build_header_err("404 File Not Found", header.length(), "text/html");
    request.ubuffer.insert(request.ubuffer.end(), buffer.begin(), buffer.end());
}

std::string	build_header_err(std::string status_code, int file_size, std::string file_type)
{
	std::ostringstream	header;

	header << "HTTP/1.1 " << status_code << CRLF;
	header << "Server: " << "Master J&J Server" << CRLF;
	header << "Connection: close" << CRLF;
	header << "Content-length: " << file_size << CRLF;
	header << "Content-type: " << file_type << CRLF;
	header << CRLF;

	return (header.str());
}

ErrorHandler::~ErrorHandler()
{}
