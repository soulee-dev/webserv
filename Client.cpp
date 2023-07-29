#include "Client.hpp"
#include "Server.hpp"

Client::Client() {}
Client::~Client() {}
Client::Client(const Client& ref)
    : port(ref.port), server(ref.server), req(ref.req), res(ref.res)
{
}
void Client::setPort(int port)
{
    this->port = port;
}

void Client::setReq(RequestMessage* req)
{
    this->req = req;
}
void Client::setServer(Server* server)
{
    this->server = server;
}

ResponseMessage* Client::getRes(void)
{
    return &this->res;
}

RequestMessage* Client::getReq(void)
{
    return this->req;
}

int Client::getPort(void)
{
    return this->port;
}

void Client::runServer()
{
    this->server->runServer(*this->req, this->res);
}
