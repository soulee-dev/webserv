#pragma once
#include "Server.hpp"
#include <fcntl.h>
#include <netdb.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

class ServerManager
{
public:
    static ServerManager getInstance()
    {
        static ServerManager instance;
        return instance;
    }
    ~ServerManager();
    typedef int SOCKET;
    typedef int PORT;
    typedef std::string serverName;

    std::map<PORT, std::vector<Server> > servers;               // init when config parsing
    std::map<PORT, std::vector<serverName> > serverNamesByPort; // init when init_server
    std::map<SOCKET, PORT> server_sockets;                      // init when config parsing

    std::vector<struct kevent> change_list;
    struct kevent event_list[8];
    void init_server();
    void start_server();
    int getKq();

private:
    ServerManager();
    void change_events(int socket, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void* udata);
    void disconnect_server(int server_fd);
    void disconnect_client(int client_fd);
    int kq;
};
