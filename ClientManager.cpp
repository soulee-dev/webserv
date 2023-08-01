#include "ClientManager.hpp"

ClientManager::ClientManager() {};
ClientManager::~ClientManager() {};

ClientManager::SOCKET ClientManager::addNewClient(SOCKET server_fd, Server* server)
{
    const int client_fd = accept(server_fd, NULL, NULL);
    if (client_fd == -1)
        return -1;
    clients[client_fd] = Client();
    clients[client_fd].setFd(client_fd);
    clients[client_fd].setServer(server);
    return client_fd;
}

void ClientManager::disconnectClient(SOCKET client_fd)
{
    clients.erase(client_fd);
}

Client& ClientManager::getClient(SOCKET client_fd)
{
    return clients[client_fd];
}