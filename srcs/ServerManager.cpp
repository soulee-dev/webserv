#include "ServerManager.hpp"

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

void ServerManager::readEventProcess(struct kevent& currEvent)
{
    if (isResponseToServer(currEvent))
        acceptClient(currEvent.ident);
    else
    {
        events.changeEvents(currEvent.ident, EVFILT_TIMER, EV_EOF, NOTE_SECONDS, 100, currEvent.udata);
        if (clientManager.readEventProcess(currEvent))
        {
            // events.changeEvents(currEvent.ident, EVFILT_READ, EV_DISABLE, 0, 0, currEvent.udata);
            events.changeEvents(currEvent.ident, EVFILT_WRITE, EV_ENABLE, 0, 0, currEvent.udata);
        }
    }
}

void ServerManager::writeEventProcess(struct kevent& currEvent)
{
    if (isResponseToServer(currEvent) == false)
    {
        // 현재 로직은 바꿔야 함 -_-; 해당 ident 가 clientFD 가 아니라는 것을 확인 하는 조건으로
        // if (&clientMaager.getClient(currEvent.ident) != NULL)
        // {
        //     clientManager.writeEventProcess(currEvent); // 더이상 보낼게 없을때 true 반환
        //     events.changeEvents(currEvent.ident, EVFILT_WRITE, EV_DISABLE, 0, 0, currEvent.udata);
        // }
        // else
        // {
            clientManager.nonClientWriteEventProcess(currEvent);
            events.changeEvents(currEvent.ident, EVFILT_WRITE, EV_DISABLE, 0, 0, currEvent.udata);
        // }
    }
    else
    {
        std::cout << "wrong way" << std::endl;
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
