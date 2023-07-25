#include "ServerManager.hpp"
#include "algorithm"
#include <iostream>
#include <sstream>

ServerManager::ServerManager()
{
    this->kq = kqueue();
    if (this->kq == -1)
    {
        std::cout << "kqueue() error" << std::endl;
        exit(1);
    }
}

ServerManager::~ServerManager()
{
    close(this->kq);
}

void ServerManager::change_events(int socket, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void* udata)
{
    struct kevent event;
    EV_SET(&event, socket, filter, flags, fflags, data, udata);
    this->change_list.push_back(event);
}

void ServerManager::disconnect_server(int server_fd)
{
    std::cout << "server disconnected: " << server_fd << std::endl;
    close(server_fd);
}

void ServerManager::disconnect_client(int client_fd, std::map<int, std::string>& clients)
{
    std::cout << "client disconnected: " << client_fd << std::endl;
    close(client_fd);
    clients.erase(client_fd);
}

int ServerManager::getKq()
{
    return this->kq;
}

void ServerManager::init_server()
{
    std::map<PORT, std::vector<Server> >::iterator it;
    struct addrinfo* info;
    struct addrinfo hint;
    struct sockaddr_in socketaddr;
    int server_socket;
    int errorcode;
    std::string strPortNumber;
    std::stringstream sstream;
    std::vector<Server>::iterator vecServerIt;

    it = this->servers.begin();
    while (it != this->servers.end())
    {
        memset(&hint, 0, sizeof(struct addrinfo));
        memset(&socketaddr, 0, sizeof(struct sockaddr_in));
        socketaddr.sin_family = AF_INET;
        std::cout << "port number : " << it->first << std::endl;
        socketaddr.sin_port = htons(it->first);
        socketaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        hint.ai_family = AF_INET;
        hint.ai_socktype = SOCK_STREAM;
        sstream << it->first;
        sstream >> strPortNumber;
        if ((errorcode = getaddrinfo(it->second.front().getServerName().c_str(), strPortNumber.c_str(), &hint, &info) != 0))
        {
            std::cout << gai_strerror(errorcode) << std::endl;
            exit(1);
        }
        server_socket = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
        if (server_socket == -1)
        {
            std::cout << "socket() error" << std::endl;
            exit(1);
        }
        if (bind(server_socket, reinterpret_cast<struct sockaddr*>(&socketaddr), sizeof(socketaddr)))
        {
            std::cout << "bind() error" << std::endl;
            exit(1);
        }
        if (listen(server_socket, 5))
        {
            std::cout << "listen() error" << std::endl;
            exit(1);
        }
        fcntl(server_socket, F_SETFL, O_NONBLOCK);
        this->server_sockets.insert(std::pair<SOCKET, PORT>(server_socket, it->first));
        serverNamesByPort.insert(std::pair<SOCKET, std::vector<serverName> >(server_socket, std::vector<serverName>()));
        vecServerIt = it->second.begin();
        while (vecServerIt != it->second.end())
        {
            serverNamesByPort[it->first].push_back(vecServerIt->getServerName());
            vecServerIt++;
        }
        change_events(server_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
        it++;
    }
}

void ServerManager::start_server()
{

}
