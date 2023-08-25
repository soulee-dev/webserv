#pragma once
#include "Client.hpp"
#include <map>
#include <set>
#include <sys/socket.h>

class Server;

class ClientManager
{
private:
    typedef int PORT;
    typedef int SOCKET;
    std::map<SOCKET, Client> clients;
    std::set<SOCKET> toDisconnectClients;
public:
    ClientManager();
    ~ClientManager();
    SOCKET addNewClient(SOCKET server_fd, Server* server, Event* event);
    void disconnectClient(SOCKET client_fd);
    Client& getClient(SOCKET client_fd);
    bool isClient(SOCKET client_fd);

    bool readEventProcess(struct kevent& currEvent);
    bool writeEventProcess(struct kevent& currEvent);
    int ReqToCgiWriteProcess(struct kevent& currEvent);
    int CgiToResReadProcess(struct kevent& currEvent);

    void clearClients(void);
    void addToDisconnectClient(SOCKET);
};
