#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Location.hpp"
#include "RequestMessage.hpp"
#include "RespondMessage.hpp"
class Client
{
private:
    Server* server;
    RequestMessage* req;
    RespondMessage res;

    Client(const Client& ref);
    Client& operator=(const Client& ref);

public:
    Client();
    Client(RequestMessage* req, Server* server);
    ~Client();
    int runServer(void);
    void setReq(RequestMessage* req);
    void setServer(Server* server);
    RespondMessage* getRes(void);
};

#endif
