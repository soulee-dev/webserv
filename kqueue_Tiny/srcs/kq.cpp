#include "../includes/Server.hpp"
#include "../includes/Color.hpp"

void exitWithPerror(const std::string &msg)
{
	std::cerr << msg << std::endl;
	exit(EXIT_FAILURE);
}

void changeEvents(std::vector<struct kevent> &change_list, uintptr_t ident,
				  int16_t filter, uint16_t flags, uint32_t fflags,
				  intptr_t data, void *udata)
{
	struct kevent _temp_event;

	EV_SET(&_temp_event, ident, filter, flags, fflags, data, udata);
	change_list.push_back(_temp_event);
}

void Server::disconnectfd(int client_fd, std::map<int, std::string> &clients)
{
	std::cout << "client disconnected" << '\n';
	std::cout << "client fd: " << client_fd << std::endl;
	close(client_fd);
	clients.erase(client_fd);
}

// std::map<int, std::string> clients;
// std::vector<struct kevent> change_list;
// struct kevent event_list[8];

void Server::ProcessTraffic(int clientfd)
{
	std::cout << BOLDMAGENTA << "reading" << RESET << std::endl;
	char buf[8192];
	struct stat stat_buf;
	Request req;
	int ret_stat;
	int n = read(clientfd, buf, sizeof(buf));
	// 이 read가 ProcessTraffic의 read인것 같다.
	// if ((received_bytes = read(fd, buf, BUFFER_SIZE)) < 0)
	// 	throw std::runtime_error("Cannot read bytes");

	if (n <= 0)
	{
		if (n < 0)
			std::cerr << " client read error! " << std::endl;
		disconnectfd(clientfd, clients);
	}
	else
	{
		changeEvents(change_list, clientfd, EVFILT_WRITE,
					 EV_ENABLE, 0, 0, NULL);
		buf[n] = '\0';
		clients[clientfd] += buf;
		std::cout << "received data from " << clientfd << " : \n\n"
				  << "--------------------------------\n"
				  << clients[clientfd]
				  << "--------------------------------\n";

		// --- process traffic --- //
		char method[BUFFER_SIZE];
		char uri[BUFFER_SIZE];
		char version[BUFFER_SIZE];

		std::sscanf(buf, "%s %s %s\r\n", method, uri, version);
		req.method = method;
		ParseURI(std::string(uri), req);
		req.fd = clientfd;
		std::cout << BOLDBLUE << "method : " << req.method << '\n';
		std::cout << BOLDBLUE << "uri : " << uri << '\n';
		std::cout << BOLDBLUE << "version : " << version << '\n';
		std::cout << BOLDBLUE << "client fd : " << clientfd << '\n';
		if (req.is_static)
		{
			ret_stat = stat(req.file_name.c_str(), &stat_buf);
			if (S_ISDIR(stat_buf.st_mode))
			{
				std::cout << req.file_name << " is dir" << std::endl;
				for (std::vector<std::string>::iterator it = index.begin(); it != index.end(); ++it)
				{
					struct stat stat_buf2;
					std::string path = req.file_name + *it;
					int ret_stat2 = stat(path.c_str(), &stat_buf2);
					if ((ret_stat2 == 0) && S_ISREG(stat_buf2.st_mode) && (S_IRUSR & stat_buf2.st_mode))
					{
						std::cout << "Serve " << path << std::endl;
						req.file_name = path;
						ServeStatic(req);
						return;
					}
				}
				if (_config["autoindex"] == "on")
				{
					ServeAutoIndex(req);
					return;
				}
			}
			if (ret_stat < 0)
			{
				std::cout << "404 ";
				ClientError(req.fd, req.file_name, "404", "Not found", "Tiny couldn't find this file");
				return;
			}
			// Check its regular file and check whether read or not;
			if (!(S_ISREG(stat_buf.st_mode)) || !(S_IRUSR & stat_buf.st_mode))
			{
				ClientError(req.fd, req.file_name, "403", "Forbidden", "Tiny couldn't read the file");
				return;
			}
			req.file_size = stat_buf.st_size;
			ServeStatic(req);
			return;
		}
	}
}

void Server::run(void)
{
	if (listen(server_socket, 20))
		throw std::runtime_error("Cannot listen socket");

	changeEvents(change_list, server_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0,
				 0, NULL);
	std::cout << BOLDGREEN << "-- Server started -- " << RESET << std::endl;

	int new_events;
	struct kevent *curr_event;
	while (1)
	{
		new_events = kevent(kq, &change_list[0], change_list.size(), event_list, 8, NULL);
		std::cout << new_events << std::endl;
		if (new_events == -1)
			exitWithPerror("kevent() error\n" + std::string(strerror(errno)));
		change_list.clear();
		for (int i = 0; i != new_events; ++i)
		{
			curr_event = &event_list[i];
			if (curr_event->flags & EV_ERROR)
			{
				if (curr_event->ident == (unsigned int)server_socket)
					exitWithPerror("server socket error");
				else
				{
					std::cerr << "client socket error" << std::endl;
					disconnectfd(curr_event->ident, clients);
				}
			}
			else if (curr_event->filter == EVFILT_READ)
			{
				if (curr_event->ident == (unsigned int)server_socket)
				{
					sleep(1);
					std::cout << BOLDYELLOW << "read" << RESET << std::endl;
					int client_socket;

					if ((client_socket = accept(server_socket, NULL, NULL)) == -1)
						exitWithPerror("accept() error\n" + std::string(strerror(errno)));
					std::cout << "accept new client : " << client_socket << std::endl;
					fcntl(client_socket, F_SETFL, O_NONBLOCK);
					changeEvents(change_list, client_socket, EVFILT_READ,
								 EV_ADD | EV_ENABLE, 0, 0, NULL);
					changeEvents(change_list, client_socket, EVFILT_WRITE,
								 EV_ADD | EV_DISABLE, 0, 0, NULL);
					clients[client_socket] = "";
					std::cout << "make buff" << std::endl;
				}
				else if (clients.find(curr_event->ident) != clients.end())
				{
					ProcessTraffic(curr_event->ident);
				}
			}
			else if (curr_event->filter == EVFILT_WRITE)
			{
				sleep(1);
				std::cout << BOLDPURPLE << "write" << RESET << std::endl;
				std::map<int, std::string>::iterator it =
					clients.find(curr_event->ident);

				if (it != clients.end())
				{
					int n;
					if ((n = write(curr_event->ident, clients[curr_event->ident].c_str(),
								   clients[curr_event->ident].size()) == -1))
					{
						std::cerr << "client write error!" << std::endl;
						disconnectfd(curr_event->ident, clients);
					}
					else
					{
						changeEvents(change_list, curr_event->ident, EVFILT_WRITE,
									 EV_DISABLE, 0, 0, NULL);
						clients[curr_event->ident].clear();
					}
				}
			}
		}
	}
}

std::string escapeControlChars(const std::string &input)
{
	std::string result;
	for (char c : input)
	{
		switch (c)
		{
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

std::string Server::BuildHeader(std::string status_code, int file_size, std::string file_type)
{
	std::ostringstream header;

	header << "HTTP/1.1 " << status_code << CRLF;
	header << "Server: " << SERVER_NAME << CRLF;
	header << "Connection: close" << CRLF;
	header << "Content-length: " << file_size << CRLF;
	header << "Content-type: " << file_type << CRLF;
	header << CRLF;

	return (header.str());
}

// Session management
// Check wheter session id setted
// If there is no seesion id, I should allocate new one.
// Ref: PHP Session
// Ref: How JESSIONID, PHPSESSID maek

// std::string	generateSessionID(size_t length)
// {
// 	const char		*char_map = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
// 								"abcdefghijklmnopqrstuvwxyz"
// 								"0123456789";
// 	const size_t
// }

std::string Server::BuildHeader(std::string status_code, int file_size, std::string file_type, std::map<std::string, std::string> cookies)
{
	std::ostringstream header;

	header << "HTTP/1.1 " << status_code << CRLF;
	header << "Server: " << SERVER_NAME << CRLF;
	header << "Connection: close" << CRLF;
	header << "Content-length: " << file_size << CRLF;
	header << "Content-type: " << file_type << CRLF;
	// For security, SESSIONID should be set as HttpOnly and Secure
	for (std::map<std::string, std::string>::iterator it = cookies.begin(); it != cookies.end(); ++it)
		header << "Set-Cookie: " << it->first << "=" << it->second << CRLF;
	header << CRLF;

	return (header.str());
}

void Server::ClientError(int fd, std::string cause, std::string error_num, std::string short_msg, std::string long_msg)
{
	std::string htmlFile = "<html><title>Tiny Error</title><body bgcolor="
						   "ffffff"
						   ">" +
						   error_num + ":" + short_msg + "<p>" + long_msg + ":" + cause + "</p> <hr><em>The Tiny Web server</em></body></html>";
	std::string message = BuildHeader(error_num + " " + short_msg, htmlFile.size(), "text/html") + htmlFile;

	ssize_t sent_bytes;

	if ((sent_bytes = send(fd, message.c_str(), message.size(), 0)) < 0)
		throw std::runtime_error("Send failed");
	if (sent_bytes == (unsigned int)message.size())
		std::cout << "Successfully send message" << std::endl;
	else
		std::cout << "Client Error Error" << std::endl;
}

extern char **environ;

void Server::ServeDynamic(Request &req)
{
	pid_t pid;
	std::ostringstream header;
	char *empty_list[] = {NULL};

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
		if (execve(req.file_name.c_str(), empty_list, environ) == -1)
		{
			std::cerr << "execve failed with error: " << strerror(errno) << std::endl;
			exit(EXIT_FAILURE);
		}
	}
	wait(NULL);
}

void Server::ServeAutoIndex(Request &req)
{
	DIR *dir = opendir(req.file_name.c_str());
	std::stringstream html_file;
	std::vector<std::string> dirs;

	html_file << "<!DOCTYPE html>";
	html_file << "<head><title>Index of " << req.path;
	html_file << "</title></head>";
	html_file << "<body>";
	html_file << "<h1>Index of " << req.path;
	html_file << "</h1><ul>";
	if (dir)
	{
		struct dirent *ent;
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
	std::string message = BuildHeader("200 OK", html_file.str().size(), "text/html") + html_file.str();

	ssize_t sent_bytes;

	if ((sent_bytes = send(req.fd, message.c_str(), message.size(), 0)) < 0)
		throw std::runtime_error("Send failed");
	if (sent_bytes == message.size())
		std::cout << "Successfully send message" << std::endl;
	else
		std::cout << "Serve Autoindex Error" << std::endl;
}

void Server::ParseURI(std::string uri, Request &req)
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
		size_t location = uri.find('?');
		if (location != std::string::npos)
			req.cgi_args = uri.substr(location + 1);
		req.file_name = "." + uri.substr(0, location);
	}
}
