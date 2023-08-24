#include "ClientManager.hpp"
#include <iostream>
#include <sys/event.h>
#include <unistd.h>
#include <vector>
#include "Message/Request.hpp"

// constructors
ClientManager::ClientManager(){};
// destructor
ClientManager::~ClientManager(){};
// copy constructors
// operators
// getter
Client& ClientManager::getClient(SOCKET client_fd)
{
    return clients.find(client_fd)->second;
}
// setter
// functions

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
    // 이거 근데 ident 로 바로 찾아와서 쓸수 있지 않은지
    Client* currClient = reinterpret_cast<Client*>(currEvent.udata);
    if (currClient->readMessage()) // read에서 에러나면 false를 반환
    {
        disconnectClient(currEvent.ident);
        return false; // read 에서 에러가 났으니 disconnect 하고 false를 반환하여 이벤트 등록하지 않도록 함.
    }
    if (currClient->readEventProcess()) // 응답을 보낼 준비가 되면 true를 반환
        return true;
    return false;
}

bool ClientManager::writeEventProcess(struct kevent& currEvent)
{
    Client* currClient = reinterpret_cast<Client*>(currEvent.udata);
    if (currClient->writeEventProcess()) // write에 실패하면 false를 반환
    {
        disconnectClient(currEvent.ident);
        return false; // write에서 에러가 났으니 disconnect 하고 false를 반환하여 이벤트 등록하지 않도록 함.
    }
    if (currClient->isSendBufferEmpty()) // Sendbuffer가 다 비워짐
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
    // if (buffer.size() == 0)
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
    const size_t BUFFER_SIZE = 65536;

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
