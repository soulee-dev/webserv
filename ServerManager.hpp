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
#include <vector>

class ServerManager
{
public:
    typedef int PORT;
    typedef int SOCKET;
    static const int LISTENCAPACITY = 5;
    ~ServerManager();
    ServerManager();
    void initServers(void);
    void exitWebServer(std::string msg);
    int openPort(ServerManager::PORT port, Server& firstServer);
    void runServerManager(void);
    int acceptClient(SOCKET server_fd); // return new client fd;
    Client& getClient(SOCKET client_fd);
    Server* getClientServer(SOCKET client_fd);

    void setServers(std::map<PORT, Server>& servers);

private:
    ServerManager(ServerManager const& other);
    ServerManager& operator=(ServerManager const& rhs);
    Event events;
    ClientManager clientManager;
    std::map<PORT, Server> servers;
    std::map<SOCKET, PORT> portByServerSocket;
};