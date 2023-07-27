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
    RespondMessage* res;

public:
    Client();
    ~Client();
    Client(const Client& ref);
    Client& operator=(const Client& ref);
    int runServer(void);
};

Client::Client() {}

Client::~Client() {}
Client::Client(const Client& ref) {}
Client& Client::operator=(const Client& ref) {}

#endif
