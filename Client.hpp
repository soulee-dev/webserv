#pragma once
#include "Location.hpp"
#include "RequestMessage.hpp"
#include "ResponseMessage.hpp"

#include "Server.hpp"
class Client
{
private:
    // int port;
    int client_fd;
    Server* server;

    RequestMessage req;
    ResponseMessage res;

    std::vector<unsigned char> readBuffer;
    std::vector<unsigned char> sendBuffer;

public:
    typedef int PORT;
    typedef int SOCKET;
    Client();
    Client(const Client& ref);
    Client& operator=(const Client& ref);
    ~Client();
    void runServer(void);
    // void setPort(int port);
    void setFd(int fd);
    void setServer(Server* server);
    ResponseMessage& getRes(void);
    RequestMessage& getReq(void);
    Server* getServer(void);
    // PORT getPort(void);
    SOCKET getClientFd(void);
};