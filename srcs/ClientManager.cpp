#include "ClientManager.hpp"
#include <sys/event.h>
#include <unistd.h>

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
ClientManager::SOCKET ClientManager::addNewClient(SOCKET client_fd, Server* server, Event *events)
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
    if (currClient->readMessage()) // read에서 에러나면 true를 반환
        disconnectClient(currEvent.ident);
    if (currClient->readEventProcess()) // 응답을 보낼 준비가 되면 true를 반환
        return true;
    return false;
}

bool ClientManager::writeEventProcess(struct kevent& currEvent)
{
    Client* currClient = reinterpret_cast<Client*>(currEvent.udata);
    if (currClient->writeEventProcess()) // write에 실패하면 true를 반환
        disconnectClient(currEvent.ident);
    if (currClient->isSendBufferEmpty())
        return true;
    return false;
}

bool ClientManager::nonClientWriteEventProcess(struct kevent& currEvent)
{

}
