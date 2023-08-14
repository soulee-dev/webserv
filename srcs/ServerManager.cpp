#include "ServerManager.hpp"
#include "Color.hpp"
#include "Http/Handler/Handler.hpp"

// constructors
ServerManager::~ServerManager(void) {}
// destructor
ServerManager::ServerManager(void) {}
// copy constructors
// operators
// getter
// setter
// functions
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
		fcntl(serversSocket, F_SETFL, O_NONBLOCK);

		// 이제 더이상 server 를 map 으로 찾을 필요가 없다면, server_socket 을 담아두는 intvector 로 담아서 써도 될듯?
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
// 이함수는 레알 진짜 격하게 고치고 싶다 ㅠ
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

void ServerManager::runEventProcess(struct kevent& currEvent) // RUN 2
{
	if (currEvent.flags & EV_ERROR)
	{
		errorEventProcess(currEvent);
		return;
	}
	switch (currEvent.filter)
	{
	case EVFILT_READ:
		readEventProcess(currEvent); // (RUN 3)
		break;
	case EVFILT_WRITE:
		writeEventProcess(currEvent);
		break;
	case EVFILT_TIMER:
		timerEventProcess(currEvent);
		break;
	}
}

void ServerManager::runServerManager(void) // RUN 1
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
			runEventProcess(events[i]); // (RUN 2)
		}
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
    fcntl(client_fd, F_SETFL, O_NONBLOCK);
    // client에 events 를 넣어야 함,
    clientManager.addNewClient(client_fd, &servers[serverPort], &events);
    events.changeEvents(client_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, &clientManager.getClient(client_fd));
    events.changeEvents(client_fd, EVFILT_WRITE, EV_ADD | EV_DISABLE, 0, 0, &clientManager.getClient(client_fd));
    events.changeEvents(client_fd, EVFILT_TIMER, EV_ADD | EV_ENABLE, NOTE_SECONDS, 100, &clientManager.getClient(client_fd));

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
		std::cout << currEvent.ident << " server disconnected" << std::endl;
	}
	else
	{
		clientManager.disconnectClient(currEvent.ident);
		std::cout << currEvent.ident << " client disconnected" << std::endl;
	}
}

bool ServerManager::isResponseToServer(struct kevent& currEvent)
{
	return portByServerSocket.find(currEvent.ident) != portByServerSocket.end();
}

void ServerManager::readEventProcess(struct kevent& currEvent) // RUN 3
{
	Client* currClient = reinterpret_cast<Client*>(currEvent.udata);

    if (isResponseToServer(currEvent))
        acceptClient(currEvent.ident);
    else if (clientManager.isClient(currEvent.ident) == true) // clinet
    {
        //Timer 설정
        events.changeEvents(currEvent.ident, EVFILT_TIMER, EV_EOF, NOTE_SECONDS, 100, currEvent.udata);
        if (clientManager.readEventProcess(currEvent))
        {
            events.changeEvents(currEvent.ident, EVFILT_WRITE, EV_ENABLE, 0, 0, currEvent.udata);
        }
    }
    else // file Read event...
    {

        //Cgi에서 보내는 data 를 response의 body 에 저장 
        ssize_t ret = clientManager.CgiToResReadProcess(currEvent); // -1: read error, 0 : read left 1 : read done
        if (ret != 0) // read error || read done
        {
            events.changeEvents(currEvent.ident, EVFILT_READ, EV_DELETE, 0, 0, NULL);
			
			close(currClient->httpRequestManager.getRequest().pipe_fd_back[0]);
        }
        if (ret == 1)
        {
            // cgi 에서 결과물을 받을때 response 가 완성 되어있다면, client 로 바로 전송 하도록 이벤트를 보냄
			currClient->sendBuffer = Handler::BuildHeader(currClient->getFrontRes().status_code, currClient->getFrontRes().headers, false);
			currClient->sendBuffer.insert(currClient->sendBuffer.end(), currClient->getFrontRes().body.begin(), currClient->getFrontRes().body.end());
			for (std::vector<unsigned char>::iterator it = currClient->sendBuffer.begin(); it != currClient->sendBuffer.end(); ++it)
				std::cout << *it;
			currClient->popRes();
            events.changeEvents(currClient->getClientFd(), EVFILT_WRITE, EV_ENABLE, 0, 0, currClient);
			wait(NULL);
            // 위의 경우가 아닌 경우에는 client 의 response 메시지를 만드는 function 을 호출한다. 
            // 예 : currClient->getRes().buildResponse();
        }
    }
}

void ServerManager::writeEventProcess(struct kevent& currEvent)
{
    if (clientManager.isClient(currEvent.ident) == true)
    {
        if (clientManager.writeEventProcess(currEvent)) // 더이상 보낼게 없을때 true 반환
            events.changeEvents(currEvent.ident, EVFILT_WRITE, EV_DISABLE, 0, 0, currEvent.udata);
    }
    else
    {
        ssize_t res = clientManager.ReqToCgiWriteProcess(currEvent);
        if (res != 0)
		{
			// close(currEvent.ident); // 아래에서 더욱 명시적으로 close를 했음
			// events.changeEvents(currEvent.ident, EVFILT_WRITE, EV_DISABLE, 0, 0, currEvent.udata); // 이미 close한 fd에 대해서 이벤트를 조정하려고 하고 있음
			Client* currClient = reinterpret_cast<Client*>(currEvent.udata);
			currClient->httpRequestManager.DynamicReadFromCgi(*currClient);
			currClient->httpRequestManager.DynamicMakeResponse(*currClient);
			close(currClient->httpRequestManager.getRequest().pipe_fd[1]);
		} // -1 : write error, 1 : buffer->size == 0, 0 : buffer left
        // BE에서 pipe fd 관리를 해주는 것이라면 여기에서 close하는게 맞나 싶음.. 중복 close로 엉뚱한 fd가 close되진 않을까..?
        // readEventProcess 에서도 동일한 이슈..
    }
}

void ServerManager::timerEventProcess(struct kevent& currEvent)
{
	events.changeEvents(currEvent.ident, EVFILT_TIMER, EV_DELETE, 0, 0, currEvent.udata);
	clientManager.disconnectClient(currEvent.ident);
}

void ServerManager::serverDisconnect(struct kevent& currEvent)
{
	close(currEvent.ident);
	servers.erase(portByServerSocket[currEvent.ident]);
}
