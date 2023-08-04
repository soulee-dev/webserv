#pragma once
#include "ClientManager.hpp"
#include "Event.hpp"
#include "Server.hpp"
#include <cstdlib>
#include <fcntl.h>
#include <iostream>
#include <map>
#include <netdb.h>
#include <sstream>
#include <unistd.h>
#include <vector>

class ServerManager
{
private:
    typedef int PORT;
    typedef int SOCKET;
    ServerManager(ServerManager const& other);
    ServerManager& operator=(ServerManager const& rhs);
    Event events;
    ClientManager clientManager;
    std::map<PORT, Server> servers;
    std::map<SOCKET, PORT> portByServerSocket;

public:
    static const int LISTENCAPACITY = 5;
    ~ServerManager();
    ServerManager();
    void initServers(void);
    void exitWebServer(std::string msg);
    int openPort(ServerManager::PORT port, Server& firstServer);
    void runServerManager(void);
    int acceptClient(SOCKET server_fd); // return new client fd;

    // getter
    Client& getClient(SOCKET client_fd);
    Server* getClientServer(SOCKET client_fd);

    // setter
    void setServers(std::map<PORT, Server>& servers);

    // functions
    void runEventProcess(struct kevent& currEvent);
    void errorEventProcess(struct kevent& currEvent);
    bool isResponseToServer(struct kevent& currEvent);
    void serverDisconnect(struct kevent& currEvent);
    void timerEventProcess(struct kevent& currEvent);
    void writeEventProcess(struct kevent& currEvent);
    void readEventProcess(struct kevent& currEvent);
};
