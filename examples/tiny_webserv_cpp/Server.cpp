#include "Server.hpp"

// class Server
// {
// 	private:
// 		std::string	_ip;
// 		int	_port;
// 		int	_socket;
// 		int	_new_socket;
// 		struct sockaddr_in	sock_addr;
// 		unsigned int	sock_addrlen;
// 	public:
// 		Server(std::string ip, int port);
// 		~Server();
// 		// void	run(std::string ip, std::string port);
// };

void exitWithError(const std::string &errorMessage)
{
	std::cout << BOLDRED << errorMessage << std::endl;
	exit(1);
}

Server::Server(std::string _ip, int _port) \
	: _ip(_ip), _port(_port), sock_addr(), sock_addrlen(sizeof(sock_addr)) \
		, _serverMessage(build_response())
{
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = htons(_port);
	sock_addr.sin_addr.s_addr = inet_addr(_ip.c_str());

	if (launch_server() != 0)
	{
		std::cout << "Failed to launch server" << ntohs(sock_addr.sin_port);
	}
}

Server::~Server() 
{
	close_server();
}

// struct sockaddr_in {
// 	__uint8_t       sin_len;
// 	sa_family_t     sin_family;
// 	in_port_t       sin_port;
// 	struct  in_addr sin_addr;
// 	char            sin_zero[8];
// };

// struct sockaddr {
// 	__uint8_t       sa_len;         /* total length */
// 	sa_family_t     sa_family;      /* [XSI] address family */
// 	char            sa_data[14];    /* [XSI] addr value (actually larger) */
// };

void Server::close_server()
{
	close(_socket);
	close(_new_socket);
	exit(0);
}

int Server::launch_server()
{
	_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket < 0)
	{
		exitWithError("Error in creating socket");
		return 1;
	}

	if (bind(_socket, (sockaddr *)&sock_addr, sock_addrlen))
	{
		exitWithError("Error in binding socket");
		return 1;
	}
	return 0;
}

void Server::Listen()
{
	if (listen(_socket, 20) < 0)
	{
		exitWithError("Error in listening socket");
	}

	std::cout << BOLDGREEN << "Listening on ADDRESS: " \
		<< inet_ntoa(sock_addr.sin_addr) << " PORT: " \
		<< ntohs(sock_addr.sin_port) << std::endl;

	int bytesReceived;
	while (true)
	{
		std::cout << BOLDYELLOW << "=== Waiting for connection ===" << std::endl;
		accept_connection(_new_socket);

		char buffer[BUFFER_SIZE];
		std::fill(buffer, buffer + BUFFER_SIZE, 0);
		bytesReceived = read(_new_socket, buffer, BUFFER_SIZE);
		if (bytesReceived < 0)
		{
			exitWithError("Failed to read bytes from client socket connection");
		}
		// parse_header(); // parse GET POST DELETE
		std::cout << BOLDRED << "--- Received Request from client ---\n";
		//do_it(_new_socket);
		send_response();
		close(_new_socket);
	}
}

void Server::accept_connection(int &new_socket)
{
	new_socket = accept(_socket, (sockaddr *)&sock_addr, &sock_addrlen); 
	if (new_socket < 0)
	{
		std::cout << BOLDRED << "Server failed to accept from ADDRESS: " \
			<< inet_ntoa(sock_addr.sin_addr) << "; PORT: " \
			<< ntohs(sock_addr.sin_port);
		exitWithError("Exited in accept_connection");
	}
}

std::string Server::build_response()
{
	std::string htmlFile = "<!DOCTYPE html><html lang=\"en\"><body><h1> HOME </h1><p> Hello from your Server :) </p></body></html>";
	std::ostringstream ss;
	ss << "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: " << htmlFile.size() << "\n\n"
		<< htmlFile;

	return ss.str();
}

void Server::send_response()
{
	long bytesSent;

	bytesSent = write(_new_socket, _serverMessage.c_str(), _serverMessage.size());
	if (bytesSent == (long) _serverMessage.size())
	{
		std::cout << BOLDBLUE << "------ Server Response sent to client ------\n\n";
	}
	else
	{
		std::cout << BOLDRED << "Error sending response to client";
	}
}