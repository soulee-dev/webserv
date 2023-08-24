#include "ClientManager.hpp"
#include "Message/Request.hpp"
#include <sys/event.h>
#include <unistd.h>
#include <vector>
#include <iostream>


ClientManager::ClientManager(){};

ClientManager::~ClientManager(){};

Client& ClientManager::getClient(SOCKET client_fd)
{
    return clients.find(client_fd)->second;
}

ClientManager::SOCKET ClientManager::addNewClient(SOCKET client_fd, Server* server, Event* events)
{
    clients[client_fd] = Client();
    clients[client_fd].setFd(client_fd);
    clients[client_fd].setServer(server);
    clients[client_fd].setEvents(events);
    return client_fd;
}

void ClientManager::disconnectClient(SOCKET client_fd)
{
    close(client_fd);
    clients.erase(client_fd);
}

bool ClientManager::readEventProcess(struct kevent& currEvent)
{
    Client* currClient = reinterpret_cast<Client*>(currEvent.udata);
    if (currClient->readMessage())
    {
        disconnectClient(currEvent.ident);
        return false;
    }
    if (currClient->readEventProcess())
        return true;
    return false;
}

bool ClientManager::writeEventProcess(struct kevent& currEvent)
{
    Client* currClient = reinterpret_cast<Client*>(currEvent.udata);
    if (currClient->writeEventProcess())
    {
        disconnectClient(currEvent.ident);
        return false;
    }
    if (currClient->isSendBufferEmpty())
        return true;
    return false;
}

int ClientManager::ReqToCgiWriteProcess(struct kevent& currEvent)
{
    Client* client = reinterpret_cast<Client*>(currEvent.udata);
    Request&    request = client->request;
    std::vector<unsigned char>& buffer = request.body;
    const int   size = buffer.size() - request.writeIndex;

    int writeSize = write(currEvent.ident, &buffer[request.writeIndex], size);
    if (writeSize == -1)
    {
        std::cout << "write() error" << std::endl;
        std::cout << "errno : " << errno << std::endl;
        return -1;
        
    }
    request.writeIndex += writeSize;
    if (request.writeIndex == buffer.size())
    {
        request.writeIndex = 0;
        buffer.clear();
        return 1;
    }
    else
        return 0;
}

int ClientManager::CgiToResReadProcess(struct kevent& currEvent)
{
    const ssize_t BUFFER_SIZE = 65536;

    Client* currClient = reinterpret_cast<Client*>(currEvent.udata);
    std::vector<unsigned char>& readBuffer = currClient->response.body;
    
    char buffer[BUFFER_SIZE];

    ssize_t ret = read(currEvent.ident, buffer, BUFFER_SIZE);
    if (ret == -1)
        return -1;
    readBuffer.insert(readBuffer.end(), buffer, &buffer[ret]);
    if ((ret == 0 || ret < BUFFER_SIZE) && currClient->request.body.size() == 0)
        return 1;
    else
        return 0;
}

bool ClientManager::isClient(SOCKET client_fd)
{
    return (clients.find(client_fd) != clients.end());
}
