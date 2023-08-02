#pragma once
#include "Client.hpp"
#include <map>
#include <sys/socket.h>

class Server;
class ClientManager
{
private:
    typedef int PORT;
    typedef int SOCKET;
    std::map<SOCKET, Client> clients;
    // ClientManager(ClientManager const& other);
    // ClientManager& operator=(ClientManager const& rhs);
public:
    ClientManager();
    ~ClientManager();
    SOCKET addNewClient(SOCKET server_fd, Server* server);
    void disconnectClient(SOCKET client_fd);
    Client& getClient(SOCKET client_fd);

    bool readEventProcess(struct kevent& currEvent);
    bool writeEventProcess(struct kevent& currEvent);
};