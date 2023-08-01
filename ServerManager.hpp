#pragma once
#include "Event.hpp"
#include "Server.hpp"
#include "ClientManager.hpp"
#include <map>
#include <vector>
#include <iostream>
#include <fcntl.h>
#include <netdb.h>
#include <cstdlib>
#include <sstream>

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

    void setServers(std::map<PORT, Server> &servers);

private:
    ServerManager(ServerManager const& other);
    ServerManager &operator=(ServerManager const &rhs);
    Event events;
    ClientManager clientManager;
    std::map<PORT, Server> servers;
    std::map<SOCKET, PORT> portByServerSocket;
};