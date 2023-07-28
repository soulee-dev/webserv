#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Location.hpp"
#include "RequestMessage.hpp"
#include "RespondMessage.hpp"
class Client
{
private:
    int port;
    Server* server;
    RequestMessage* req;
    RespondMessage res;

    Client& operator=(const Client& ref);

public:
    Client();
    Client(RequestMessage* req, Server* server);
    Client(const Client& ref);
    ~Client();
    void runServer(void);
    void setPort(int port);
    void setReq(RequestMessage* req);
    void setServer(Server* server);
    RespondMessage* getRes(void);
    RequestMessage* getReq(void);
    int getPort(void);
};

#endif
