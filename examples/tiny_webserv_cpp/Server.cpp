#include "Server.hpp"
#include "request.hpp"

Server::Server(std::string ip_addr, unsigned int port) : _ip_addr(ip_addr), _port(port), _sock_addr_len(sizeof(_sock_addr))
{
	int opt = 1;

	_sock_addr.sin_family = AF_INET;
	// Translate short into big-endian
	_sock_addr.sin_port = htons(_port);
	// Translate into big-endian
	_sock_addr.sin_addr.s_addr = inet_addr(_ip_addr.c_str());
	// AF_INET: set socket as IPv4
	// SOCK_STREAM: set socket as TCP
	if ((_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		throw std::runtime_error("Cannot create socket");
	setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	if (bind(_socket, (sockaddr *)&_sock_addr, _sock_addr_len) < 0)
		throw std::runtime_error("Cannot bind socket to address");
	
	_config["root"] = "/html";
	index.push_back("index.html");
	index.push_back("index.htm");
	// _config["index"] = "index.html"; // vector, index.html, index.htm, directory
	_config["autoindex"] = "on";
}

Server::~Server()
{
	close(_socket);
	// close(_new_socket);
}

void	Server::ParseURI(std::string uri, Request &req)
{
	if (uri.find("cgi-bin") == std::string::npos)
	{
		req.is_static = true;
		req.path = uri;
		req.file_name += "." + _config["root"] + uri; //./index.html
		if (uri[uri.length() - 1] == '/')
			req.is_directory = true;
	}
	else
	{
		req.is_static = false;
		size_t	location = uri.find('?');
		if (location != std::string::npos)
			req.cgi_args = uri.substr(location + 1);
		req.file_name = "." + uri.substr(0, location);
	}
}

void	Server::ProcessTraffic(int fd)
{
	char		method[BUFFER_SIZE];
	char		uri[BUFFER_SIZE];
	char		version[BUFFER_SIZE];
	ssize_t		received_bytes;
	struct stat	stat_buf;
	char		buf[BUFFER_SIZE] = {0};
	Request		req;
	int			ret_stat;
	
	if ((received_bytes = read(fd, buf, BUFFER_SIZE)) < 0)
		throw std::runtime_error("Cannot read bytes");
	std::sscanf(buf, "%s %s %s\r\n", method, uri, version);
	req.method = method;
	req.fd = fd;
	ParseURI(std::string(uri), req);
	if (req.is_static)
	{
		ret_stat = stat(req.file_name.c_str(), &stat_buf);
		if (S_ISDIR(stat_buf.st_mode))
		{
			std::cout << req.file_name << " is dir" << std::endl;
			for (std::vector<std::string>::iterator it = index.begin(); it != index.end(); ++it)
			{
				struct stat	stat_buf2;
				std::string	path = req.file_name + *it;
				int	ret_stat2 = stat(path.c_str(), &stat_buf2);
				if ((ret_stat2 == 0) && S_ISREG(stat_buf2.st_mode) && (S_IRUSR & stat_buf2.st_mode))
				{
					std::cout << "Serve " << path << std::endl;
					req.file_name = path;
					ServeStatic(req);
					return ;
				}
			}
			if (_config["autoindex"] == "on")
			{
				ServeAutoIndex(req);
				return ;
			}
		}
		if (ret_stat < 0)
		{
			std::cout << "404 ";
			ClientError(req.fd, req.file_name, "404", "Not found", "Tiny couldn't find this file");
			return ;
		}
		// Check its regular file and check whether read or not;
		if (!(S_ISREG(stat_buf.st_mode)) || !(S_IRUSR & stat_buf.st_mode))
		{
			ClientError(req.fd, req.file_name, "403", "Forbidden", "Tiny couldn't read the file");
			return ;
		}
		req.file_size = stat_buf.st_size;
		ServeStatic(req);
		return ;
	}
	ServeDynamic(req);
}

void	Server::ServeAutoIndex(Request& req)
{
	DIR							*dir = opendir(req.file_name.c_str());
	std::stringstream			html_file;
	std::vector<std::string>	dirs;

	html_file << "<!DOCTYPE html>";
	html_file << "<head><title>Index of " << req.path;
	html_file << "</title></head>";
	html_file << "<body>";
	html_file << "<h1>Index of " << req.path;
	html_file << "</h1><ul>";
	if (dir)
	{
		struct dirent	*ent;
 		while ((ent = readdir(dir)) != NULL)
		{
			html_file << "<li><a href=";
			html_file << req.path << "/";
			html_file << ent->d_name;
			html_file << ">";
			html_file << ent->d_name;
			html_file << "</a></li>";
		}
		closedir(dir);
	}
	else
	{
		// NO DIRECTORY
		// NOT IMPLEMENTED
		std::cout << "Could not open dir" << std::endl;
	}
	html_file << "</ul></body></html>";
    std::string	message = BuildHeader("200 OK", html_file.str().size(), "text/html") + html_file.str();

	ssize_t	sent_bytes;

	if ((sent_bytes = send(req.fd, message.c_str(), message.size(), 0)) < 0)
		throw std::runtime_error("Send failed");
	if (sent_bytes == message.size())
		std::cout << "Successfully send message" << std::endl;
	else
		std::cout << "Error" << std::endl;
}

void	Server::run()
{
	int	_new_socket;

	// Second parameter is backlog (maximum length for the queue of pending connects)
	if (listen(_socket, 20))
		throw std::runtime_error("Cannot listen socket");
	while (true)
	{
		std::cout << "Waiting for a new connection" << std::endl;
		if ((_new_socket = accept(_socket, (sockaddr *)&_sock_addr, &_sock_addr_len)) < 0)
			throw std::runtime_error("Cannot accept incoming connection");
		ProcessTraffic(_new_socket);
		close(_new_socket);
	}
}

std::string escapeControlChars(const std::string& input) {
    std::string result;
    for (char c : input) {
        switch (c) {
            case '\n':
                result += "\\n";
                break;
            case '\r':
                result += "\\r";
                break;
            default:
                result += c;
                break;
        }
    }
    return result;
}

std::string	Server::BuildHeader(std::string status_code, int file_size, std::string file_type)
{
	std::ostringstream	header;

	header << "HTTP/1.1 " << status_code << CRLF;
	header << "Server: " << SERVER_NAME << CRLF;
	header << "Connection: close" << CRLF;
	header << "Content-length: " << file_size << CRLF;
	header << "Content-type: " << file_type << CRLF;
	header << CRLF;

	return (header.str());
}

void	Server::ClientError(int fd, std::string cause, std::string error_num, std::string short_msg, std::string long_msg)
{
	std::string htmlFile = "<html><title>Tiny Error</title><body bgcolor=""ffffff"">" + error_num + ":" + short_msg + "<p>" + long_msg + ":" + cause + "</p> <hr><em>The Tiny Web server</em></body></html>";
    std::string	message = BuildHeader(error_num + " " + short_msg, htmlFile.size(), "text/html") + htmlFile;

	ssize_t	sent_bytes;

	if ((sent_bytes = send(fd, message.c_str(), message.size(), 0)) < 0)
		throw std::runtime_error("Send failed");
	if (sent_bytes == message.size())
		std::cout << "Successfully send message" << std::endl;
	else
		std::cout << "Error" << std::endl;
}

std::string	getFileType(std::string file_name)
{
	std::string	file_type;

	if (file_name.find(".html") != std::string::npos || file_name.find(".htm") != std::string::npos)
		file_type = "text/html";
	else if (file_name.find(".gif") != std::string::npos)
		file_type = "image/gif";
	else if (file_name.find(".png") != std::string::npos)
		file_type = "image/png";
	else if (file_name.find(".jpg") != std::string::npos)
		file_type = "image/jpeg";
	else if (file_name.find(".mpg") != std::string::npos)
		file_type = "video/mpg";
	else if (file_name.find(".mp4") != std::string::npos)
		file_type = "video/mp4";
	else
		file_type = "text/plain";
	return (file_type);
}

void	Server::ServeStatic(Request& req)
{
	std::string	file_type = getFileType(req.file_name);
	ssize_t	sent_bytes;
	std::vector<char> buffer;
	std::string header;
	std::ifstream file(req.file_name, std::ios::binary);

	// get length of file:
	file.seekg(0, file.end);
	int length = file.tellg();
	file.seekg(0, file.beg);
	buffer.resize(length);
	file.read(&buffer[0], length);
	header = BuildHeader("200 OK", length, file_type);
	std::vector<char> response(header.begin(), header.end());
	response.insert(response.end(), buffer.begin(), buffer.end());
	if ((sent_bytes = send(req.fd, &response[0], response.size(), 0)) < 0)
		throw std::runtime_error("Send failed");
	if (sent_bytes == response.size())
		std::cout << "Successfully send message" << std::endl;
	else
		std::cout << "Error" << std::endl;
}

extern char **environ;

void	Server::ServeDynamic(Request& req)
{
	pid_t pid;
	std::ostringstream	header;
	char				*empty_list[] = { NULL };

	header << "HTTP/1.1 200 OK" << CRLF;
	header << "Server: " << SERVER_NAME << CRLF;
	if (send(req.fd, header.str().c_str(), header.str().size(), 0) < 0)
		throw std::runtime_error("Send failed");

	std::cout << "Attempting to execve: " << req.file_name.c_str() << std::endl;
	pid = fork();
	if (pid < 0)
		throw std::runtime_error("Fork failed");
	if (pid == 0)
	{
		setenv("QUERY_STRING", req.cgi_args.c_str(), 1);
		setenv("REQUEST_METHOD", req.method.c_str(), 1);
		dup2(req.fd, STDOUT_FILENO);
		if(execve(req.file_name.c_str(), empty_list, environ) == -1) 
		{
			std::cerr << "execve failed with error: " << strerror(errno) << std::endl;
			exit(EXIT_FAILURE);
		}
	}
	wait(NULL);
}