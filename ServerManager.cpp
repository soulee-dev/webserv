#include "ServerManager.hpp"

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
        fcntl(serversSocket, F_SETFL, O_NONBLOCK);

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

    memset(&hint, 0, sizeof(struct addrinfo));
    memset(&socketaddr, 0, sizeof(struct sockaddr_in));
    socketaddr.sin_family = AF_INET;
    std::cout << "Port number : " << port << std::endl;

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
            struct kevent& currEvent = events[i];

            if (currEvent.flags & EV_ERROR)
            {
                // errorEventProcess(currEvent.ident);
                continue;
            }
            switch (currEvent.filter)
            {
            case EVFILT_READ:
                // readEventProcess(currEvent.ident);
                break;
            case EVFILT_WRITE:
                // writeEventProcess(currEvent.ident);
                break;
            case EVFILT_TIMER:
                // timerEventProcess(currEvent.ident);
                break;
            }
        }
    }
}

int ServerManager::acceptClient(SOCKET server_fd)
{
    const int client_fd = accept(server_fd, NULL, NULL);
    const int serverPort = portByServerSocket[server_fd];
    if (client_fd == -1)
        return -1;
    clientManager.addNewClient(server_fd, &servers[serverPort]);
    events.changeEvents(client_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, &clientManager.getClient(client_fd));
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
