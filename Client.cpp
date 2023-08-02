#include "Client.hpp"
#include "Server.hpp"

Client::Client() {}
Client::~Client() {}
Client::Client(Client const& other)
    : client_fd(other.client_fd), server(other.server), req(other.req), res(other.res),
      readBuffer(other.readBuffer), sendBuffer(other.sendBuffer) {}
Client& Client::operator=(Client const& rhs)
{
    if (this != &rhs)
    {
        this->res = rhs.res;
        this->req = rhs.req;
        this->client_fd = rhs.client_fd;
        this->server = rhs.server;
    }
    return *this;
}

void Client::setServer(Server* server)
{
    this->server = server;
}

ResponseMessage& Client::getRes(void)
{
    return this->res;
}

RequestMessage& Client::getReq(void)
{
    return this->req;
}

void Client::setFd(int fd)
{
    this->client_fd = fd;
}

void Client::runServer()
{
    this->server->runServer(this->req, this->res);
}

Server* Client::getServer(void)
{
    return this->server;
}