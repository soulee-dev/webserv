#include "ServerManager.hpp"
#include "Color.hpp"
#include "Http/Handler/Handler.hpp"
#include "Http/Handler/HttpStatusCodes.hpp"
#include <map>
#include <vector>

ServerManager::~ServerManager(void) {}

ServerManager::ServerManager(void) {}

void ServerManager::initServers(void)
{
	std::map<PORT, Server>::iterator portIter = servers.begin();

	if (events.initKqueue())
	{
		std::cout << "kqueue() error" << std::endl;
		exit(1);
	}
	while (portIter != servers.end())
	{
		Server& server = portIter->second;
		SOCKET serversSocket = openPort(portIter->first, server);
		fcntl(serversSocket, F_SETFL, O_NONBLOCK, FD_CLOEXEC);
		portByServerSocket[serversSocket] = portIter->first;
		events.changeEvents(serversSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, &portIter->second);
		portIter++;
	}
}

static std::string intToString(int number)
{
	std::stringstream sstream(number);
	return sstream.str();
}

int ServerManager::openPort(ServerManager::PORT port, Server& server)
{
	struct addrinfo* info;
	struct addrinfo hint;
	struct sockaddr_in socketaddr;
	int opt = 1;

	std::cout << "Port number : " << port << std::endl;

	memset(&hint, 0, sizeof(struct addrinfo));
	memset(&socketaddr, 0, sizeof(struct sockaddr_in));

	socketaddr.sin_family = AF_INET;
	socketaddr.sin_port = htons(port);
	socketaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	hint.ai_family = AF_INET;
	hint.ai_socktype = SOCK_STREAM;

	std::string strPortNumber = intToString(port);

	int errorCode = getaddrinfo(server.getServerName().c_str(), strPortNumber.c_str(), &hint, &info);
	if (errorCode == -1)
		exitWebServer(gai_strerror(errorCode));

	SOCKET serverSocket = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
	if (serverSocket == -1)
		exitWebServer("socket() error");
	setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	errorCode = bind(serverSocket, reinterpret_cast<struct sockaddr*>(&socketaddr), sizeof(socketaddr));
	if (errorCode)
		exitWebServer("bind() error");
	errorCode = listen(serverSocket, LISTENCAPACITY);
	if (errorCode)
		exitWebServer("listen() error");
	return serverSocket;
}

void ServerManager::exitWebServer(std::string msg)
{
	std::cout << msg << std::endl;
	exit(1);
}

void ServerManager::runEventProcess(struct kevent& currEvent)
{
	if (currEvent.flags & EV_ERROR)
	{
		errorEventProcess(currEvent);
		return;
	}
	switch (currEvent.filter)
	{
	case EVFILT_READ:
		readEventProcess(currEvent);
		break;
	case EVFILT_WRITE:
		writeEventProcess(currEvent);
		break;
	case EVFILT_TIMER:
		timerEventProcess(currEvent);
		break;
	}
}

void ServerManager::runServerManager(void)
{
	int newEvent;

	while (1)
	{
		newEvent = events.newEvents();
		if (newEvent == -1)
			exitWebServer("kevent() error");
		events.clearChangeEventList();
		for (int i = 0; i != newEvent; i++)
		{
			runEventProcess(events[i]);
		}
		clientManager.clearClients();
	}
}

int ServerManager::acceptClient(SOCKET server_fd)
{
    const int client_fd = accept(server_fd, NULL, NULL);
    const int serverPort = portByServerSocket[server_fd];
    if (client_fd == -1)
    {
        std::cout << "accept() error" << std::endl;
        return -1;
    }
    fcntl(client_fd, F_SETFL, O_NONBLOCK, FD_CLOEXEC);
    clientManager.addNewClient(client_fd, &servers[serverPort], &events);
    events.changeEvents(client_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, &clientManager.getClient(client_fd));
    events.changeEvents(client_fd, EVFILT_WRITE, EV_ADD | EV_DISABLE, 0, 0, &clientManager.getClient(client_fd));
	return client_fd;
}

Client& ServerManager::getClient(SOCKET client_fd)
{
	return clientManager.getClient(client_fd);
}

Server* ServerManager::getClientServer(SOCKET client_fd)
{
	return clientManager.getClient(client_fd).getServer();
}

void ServerManager::setServers(std::map<PORT, Server>& servers)
{
	this->servers = servers;
}

ServerManager::ServerManager(ServerManager const& other)
{
	static_cast<void>(other);
}

ServerManager& ServerManager::operator=(ServerManager const& rhs)
{
	static_cast<void>(rhs);
	return *this;
}

void ServerManager::errorEventProcess(struct kevent& currEvent)
{
	if (isResponseToServer(currEvent))
	{
		serverDisconnect(currEvent);
		std::cout << BOLDMAGENTA << currEvent.ident << "SERVER DISCONNECTED" << std::endl;
	}
	else
	{
		clientManager.addToDisconnectClient(currEvent.ident);
		// std::cout << BOLDMAGENTA << currEvent.ident << " CLIENT DISCONNECTED" << std::endl;
	}
}

bool ServerManager::isResponseToServer(struct kevent& currEvent)
{
	return portByServerSocket.find(currEvent.ident) != portByServerSocket.end();
}

void ServerManager::readEventProcess(struct kevent& currEvent)
{
	Client* currClient = reinterpret_cast<Client*>(currEvent.udata);

    if (isResponseToServer(currEvent))
        acceptClient(currEvent.ident);
	else if (clientManager.isClient(currEvent.ident) == true)
    {
		if (currEvent.flags & EV_EOF)
		{
			std::cout << BOLDRED << "EV_EOF detacted in client\n" << RESET << std::endl;
			clientManager.addToDisconnectClient(currEvent.ident);
		}
		else if (clientManager.readEventProcess(currEvent))
            events.changeEvents(currEvent.ident, EVFILT_WRITE, EV_ENABLE, 0, 0, currEvent.udata);
    }
    else
    {
        ssize_t ret = clientManager.CgiToResReadProcess(currEvent);
        if (ret != 0)
		{
			close(currEvent.ident); // Dynamic 일때는  pipe를 닫아주는 close // Static일때는 파일의 fd를 닫아줌
		}
        if (ret == 1)
        {
			std::vector<unsigned char> empty_body;
			if (currClient->request.method == "PUT")
				currClient->response.body = empty_body;
			else if (currClient->request.is_static == false)
			{
				currClient->response.headers["Connection"] = "close";
				SetResponse(*currClient, 200, currClient->response.headers, currClient->response.body);
			}
			// --- //
			std::cout << "> " << currClient->request.method << ' ' << currClient->request.uri << ' ' << currClient->request.http_version << '\n';
			std::cout << "> Host: " << currClient->request.headers["host"] << '\n';
			std::cout << "> User-Agent: " << currClient->request.headers["user-agent"] << '\n';
			std::cout << "> Accept: */*\n";
			std::cout << "> Content-Length: " << currClient->response.body.size() << '\n';
			std::cout << "> Content-Type: " << currClient->request.headers["content-type"] << "\n\n";

			std::cout << "< " << currClient->request.http_version << ' ' << currClient->response.status_code \
				<< ' ' << get_status_codes()[currClient->response.status_code] << '\n' << "< Server: Master J&J Server\n" \
					<< "< Content-Length: " << currClient->response.body.size() << "\n\n";
			// --- //
			currClient->sendBuffer = BuildResponse(currClient->response.status_code, currClient->response.headers, currClient->response.body, currClient->request.is_static);
			events.changeEvents(currClient->getClientFd(), EVFILT_WRITE, EV_ENABLE, 0, 0, currClient);
			if (!currClient->request.is_static)
				wait(NULL);
        }
    }
}

void ServerManager::writeEventProcess(struct kevent& currEvent)
{
    if (clientManager.isClient(currEvent.ident) == true)
    {
        if (clientManager.writeEventProcess(currEvent))
		{
            events.changeEvents(currEvent.ident, EVFILT_WRITE, EV_DISABLE, 0, 0, currEvent.udata);
			events.changeEvents(currEvent.ident, EVFILT_READ, EV_ENABLE, 0, 0, currEvent.udata);
		}
    }
    else
    {
        ssize_t res = clientManager.ReqToCgiWriteProcess(currEvent);
        if (res != 0)
		{
			Client* currClient = reinterpret_cast<Client*>(currEvent.udata);
			close(currClient->request.pipe_fd[1]);
		} 
    }
}

void ServerManager::timerEventProcess(struct kevent& currEvent)
{
	clientManager.addToDisconnectClient(currEvent.ident);
}

void ServerManager::serverDisconnect(struct kevent& currEvent)
{
	close(currEvent.ident);
	servers.erase(portByServerSocket[currEvent.ident]);
}
