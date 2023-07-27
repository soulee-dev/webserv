#include "Client.hpp"
#include "Server.hpp"

Client::Client() {}
Client::~Client() {}
void Client::setReq(RequestMessage* req)
{
    this->req = req;
}
void Client::setServer(Server* server)
{
    this->server = server;
}

RespondMessage* Client::getRes(void)
{
    return &this->res;
}

void Client::runServer()
{
    this->server->runServer(*this->req, this->res);
}
