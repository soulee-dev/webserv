#include "ServerManager.hpp"
#include "ResponseMessageWriter.hpp"
#include "Server.hpp"
#include <cstdlib>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <sstream>
#include <sys/socket.h>
#include <vector>

void ServerManager::exitWebServer(std::string str)
{
    std::cout << str << std::endl;
    exit(1);
}

ServerManager ServerManager::getInstance()
{
    static ServerManager instance;
    return instance;
}

ServerManager::ServerManager()
    : LISTENCAPACITY(5)
{
    events.setKq();
    messageReader = &RequestMessageReader::getInstance();
    messageWriter = &ResponseMessageWriter::getInstance();
}

ServerManager::~ServerManager()
{
    close(events.getKq());
}

void ServerManager::disconnectServer(int serverFd)
{
    std::cout << "Server disconnected: " << serverFd << std::endl;
    close(serverFd);
}

void ServerManager::disconnectClient(int clientFd)
{
    std::cout << "Server disconnected: " << clientFd << std::endl;
    close(clientFd);
}

Server* ServerManager::getClientServer(SOCKET ident)
{
    Server* res = NULL;

    Client& currClient = clientsBySocket[ident];
    PORT currPort = currClient.getPort();
    serverName clientHost = currClient.getReq()->headers["host"];

    std::vector<Server>::iterator iter = servers[currPort].begin();

    if (iter == servers[currPort].end())
        return res;
    res = &servers[currPort][0];
    while (iter != servers[currPort].end())
    {
        if (iter->getServerName().compare(clientHost))
        {
            res = &(*iter);
        }
        iter++;
    }
    return res;
}

void ServerManager::insertClient(SOCKET ident)
{
    clientsBySocket.insert(std::pair<SOCKET, Client>(ident, Client()));
}

static std::string intToString(int n)
{
    std::string res;
    std::stringstream sstream;

    sstream << n;
    sstream >> res;
    return res;
}

int ServerManager::openPort(ServerManager::PORT port, Server& firstServer)
{
    struct addrinfo* info;
    struct addrinfo hint;
    struct sockaddr_in socketaddr;

    memset(&hint, 0, sizeof(struct addrinfo));
    memset(&socketaddr, 0, sizeof(struct sockaddr_in));
    socketaddr.sin_family = AF_INET;
    std::cout << "Port number : " << port << std::endl;

    socketaddr.sin_port = htons(port);
    socketaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    hint.ai_family = AF_INET;
    hint.ai_socktype = SOCK_STREAM;

    std::string strPortNumber = intToString(port);
    int errorCode = getaddrinfo(firstServer.getServerName().c_str(), strPortNumber.c_str(), &hint, &info);
    if (errorCode == -1)
        exitWebServer(gai_strerror(errorCode));

    SOCKET serverSocket = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
    if (serverSocket == -1)
        exitWebServer("socket() error");

    errorCode = bind(serverSocket, reinterpret_cast<struct sockaddr*>(&socketaddr), sizeof(socketaddr));
    if (errorCode)
        exitWebServer("bind() error");
    errorCode = listen(serverSocket, LISTENCAPACITY);
    if (errorCode)
        exitWebServer("listen() error");
    return serverSocket;
}

void ServerManager::initServers(void)
{
    std::map<PORT, std::vector<Server> >::iterator portIter = servers.begin();

    while (portIter != servers.end())
    {
        // std::vector<Server>::iterator serverIter = portIter->second.begin();
        Server& firstServer = portIter->second.front();
        SOCKET serversSocket = openPort(portIter->first, firstServer);
        fcntl(serversSocket, F_SETFL, O_NONBLOCK);

        portByServerSocket[serversSocket] = portIter->first;
        events.changeEvents(serversSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
        // udata 에 portIter->second 넣어도 될듯?
        portIter++;
    }
}

void ServerManager::errorEventProcess(SOCKET ident)
{
    if (portByServerSocket.find(ident) != portByServerSocket.end())
    {
        std::cout << "Server socket error" << std::endl;
        disconnectServer(ident);
    }
    else
    {
        std::cout << "Client socket error" << std::endl;
        disconnectClient(ident);
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
            struct kevent& currEvent = events[i];

            if (currEvent.flags & EV_ERROR)
                errorEventProcess(currEvent.ident);
            else if (currEvent.filter == EVFILT_READ)
            {
            }
            else if (currEvent.filter == EVFILT_WRITE)
            {
            }
            else if (currEvent.filter == EVFILT_TIMER)
            {
            }
        }
    }
}
