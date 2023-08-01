#include "Client.hpp"
#include "Server.hpp"

Client::Client() {}
Client::~Client() {}
// void Client::setPort(int port)
// {
//     this->port = port;
// }

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

// int Client::getPort(void)
// {
//     return this->port;
// }

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