#pragma once
#include "Client.hpp"
#include "Event.hpp"
#include "RequestMessageReader.hpp"
#include "ResponseMessageWriter.hpp"
#include "Server.hpp"
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

class ServerManager
{
public:
    const int LISTENCAPACITY;
    RequestMessageReader* messageReader;
    ResponseMessageWriter* messageWriter;
    typedef int SOCKET;
    typedef int PORT;
    typedef std::string serverName; //???

    static ServerManager getInstance();
    ~ServerManager();

    void initServers();
    void start_server();
    void exitWebServer(std::string str);

    int openPort(ServerManager::PORT port, Server& firstServer);

    void runServerManager(void);

    void insertClient(SOCKET ident);
    Client& getClient(SOCKET ident);
    Server* getClientServer(SOCKET ident);

private:
    Event events;

    std::map<PORT, std::vector<Server> > servers; // init when config parsing
    // std::map<PORT, std::vector<serverName> > serverNamesByPort; // init when init_server
    std::map<SOCKET, PORT> portByServerSocket; // init when config parsing
    std::map<SOCKET, Client> clientsBySocket;

    ServerManager();
    void disconnectServer(int server_fd);
    void disconnectClient(int client_fd);

    void errorEventProcess(SOCKET ident);
    void readEventProcess(void);
    void writeEventProcess(void);
    void timerEventProcess(void);
};
