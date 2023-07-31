#include "ServerManager.hpp"
#include "ResponseMessageWriter.hpp"
#include "Server.hpp"
#include <cstdlib>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

void ServerManager::setServers(std::map<PORT, std::vector<Server> >& servers)
{
    this->servers = servers;
}

void ServerManager::exitWebServer(std::string str)
{
    std::cout << str << std::endl;
    exit(1);
}

ServerManager& ServerManager::getInstance()
{
    static ServerManager instance;
    return instance;
}

ServerManager::ServerManager()
    : LISTENCAPACITY(5)
{
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
    events.changeEvents(serverFd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
    close(serverFd);
}

void ServerManager::disconnectClient(int clientFd)
{
    messageReader->deleteClient(clientFd);
    messageWriter->deleteClient(clientFd);
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
        if (iter->getServerName().compare(clientHost) == 0)
        {
            res = &(*iter);
            break;
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
    int errorCode = getaddrinfo(firstServer.getServerName().c_str(), strPortNumber.c_str(), &hint, &info);
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

void ServerManager::initServers(void)
{
    std::map<PORT, std::vector<Server> >::iterator portIter = servers.begin();

    if (events.initKqueue())
    {
        std::cout << "kqueue() error" << std::endl;
        exit(1);
    }
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

std::string BuildHeader(std::string status_code, int file_size, std::string file_type)
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

void ServerManager::readEventProcess(SOCKET ident)
{
    if (isRespondToServer(ident))
        acceptClient(ident);
    else
    {
        events.changeEvents(ident, EVFILT_TIMER, EV_EOF, NOTE_SECONDS, 1000, NULL);
        if (messageReader->readMessage(ident))
            disconnectClient(ident);
        else if (messageReader->ParseState[ident] == DONE || messageReader->ParseState[ident] == ERROR)
        {
            // 메시지 처리하여 버퍼에 입력해야함.
            // events.changeEvents(ident, EVFILT_WRITE, EV_ENABLE, 0, 0, NULL);
            std::cout << "메시지 잘 받았습니다^^" << std::endl;

            RequestMessage  req = messageReader->getInstance().messageBuffer[ident];

            std::string     file_type = "text/html";
	        std::vector<char>   buffer;
            std::string     header;
            std::ifstream   file("." + req.requestTarget + "index.html", std::ios::binary);

            file.seekg(0, file.end);
            int length = file.tellg();
            file.seekg(0, file.beg);
            buffer.resize(length);
            file.read(&buffer[0], length);
            header = BuildHeader("200 OK", length, file_type);
	        std::vector<char> response(header.begin(), header.end());
	        response.insert(response.end(), buffer.begin(), buffer.end());
            for (std::vector<char>::iterator it = response.begin(); it != response.end(); ++it)
            {
                std::cout << *it;
            }
            std::cout << std::endl;
            messageWriter->writeBuffer[ident].insert(messageWriter->writeBuffer[ident].end(), response.begin(), response.end());
            events.changeEvents(ident, EVFILT_WRITE, EV_ENABLE, 0, 0, NULL);
            messageReader->ParseState[ident] = METHOD;
            messageReader->messageBuffer[ident].clear();
        }
    }
}

void ServerManager::writeEventProcess(SOCKET ident)
{
    size_t n;
    n = write(ident, &*messageWriter->writeBuffer[ident].begin(), messageWriter->writeBuffer[ident].size());
    events.changeEvents(ident, EVFILT_WRITE, EV_DISABLE, 0, 0, NULL);
}

void ServerManager::timerEventProcess(SOCKET ident)
{
    events.changeEvents(ident, EVFILT_TIMER, EV_DELETE, 0, 0, NULL);
    disconnectClient(ident);
}

bool ServerManager::isRespondToServer(SOCKET serverSocket)
{
    return portByServerSocket.find(serverSocket) != portByServerSocket.end();
}

void ServerManager::acceptClient(SOCKET serverSocket)
{
    const int clientSocket = accept(serverSocket, NULL, NULL);
    if (clientSocket == -1)
    {
        std::cout << "accept() error" << std::endl;
        return;
    }
    fcntl(clientSocket, F_SETFL, O_NONBLOCK);
    events.changeEvents(clientSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
    events.changeEvents(clientSocket, EVFILT_TIMER, EV_ADD | EV_ENABLE, NOTE_SECONDS, 1000, NULL);
    events.changeEvents(clientSocket, EVFILT_WRITE, EV_ADD | EV_DISABLE, 0, 0, NULL);
    messageReader->insertNewClient(clientSocket);
    messageWriter->insertNewClient(clientSocket);
    insertClient(clientSocket);
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
                errorEventProcess(currEvent.ident);
                continue;
            }
            switch (currEvent.filter)
            {
            case EVFILT_READ:
                readEventProcess(currEvent.ident);
                break;
            case EVFILT_WRITE:
                writeEventProcess(currEvent.ident);
                break;
            case EVFILT_TIMER:
                timerEventProcess(currEvent.ident);
                break;
            }
        }
    }
}
