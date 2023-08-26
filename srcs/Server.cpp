#include "Server.hpp"
#include "Client.hpp"
#include "Utils.hpp"
#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

// constructors
Server::Server()
    : _listen(80), serverName(""), root("/"), autoIndex(false), clientBodySize(10240) {}
// destructor
Server::~Server() {}
// copy constructors

Server::Server(Server const& other) : _listen(other._listen), serverName(other.serverName),
                                      root(other.root), errorPage(other.errorPage), redirection(other.redirection),
                                      autoIndex(other.autoIndex), clientBodySize(other.clientBodySize), locations(other.locations) {}
// operators
Server& Server::operator=(Server const& rhs)
{
    _listen = rhs._listen;
    serverName = rhs.serverName;
    root = rhs.root;
    errorPage = rhs.errorPage;
    redirection = rhs.redirection;
    autoIndex = rhs.autoIndex;
    clientBodySize = rhs.clientBodySize;
    locations = rhs.locations;
    return *this;
}

// getter
int Server::getListen() const
{
    return this->_listen;
}

std::map<std::string, Location> Server::getLocations(void) const
{
    return this->locations;
}

std::string Server::getServerName(void) const
{
    return this->serverName;
}

std::string Server::getRoot(void) const
{
    return this->root;
}

std::map<std::vector<int>, std::string> Server::getErrorPage(void) const
{
    return this->errorPage;
}

std::map<std::string, int> Server::getRedirection(void) const
{
    return this->redirection;
}

bool Server::getAutoIndex(void) const
{
    return this->autoIndex;
}

unsigned int Server::getClientBodySize(void) const
{
    return this->clientBodySize;
}

// setter
bool Server::fillServer(std::map<std::string, Location>& mapLocations, std::map<std::string, std::string>& mapSentence)
{
    std::map<std::string, std::string>::iterator it = mapSentence.begin();

    this->locations = mapLocations;
    while (it != mapSentence.end())
    {
        if (it->first == "listen")
            this->setListen(it->second);
        else if (it->first == "server_name")
            this->setServerName(it->second);
        else if (it->first == "root")
            this->setRoot(it->second);
        else if (it->first == "error_page")
        {
            if (this->setErrorPage(it->second))
                return true;
        }
        else if (it->first == "return")
        {
            if (this->setRedirection(it->second))
                return true;
        }
        else if (it->first == "autoindex")
            this->setAutoIndex(it->second);
        else if (it->first == "client_max_body_size")
            this->setClientBodySize(it->second);
        else
            return true;
        it++;
    }
    return false;
}

void Server::setListen(std::string& input)
{
    long parsedPort = strtod(input.c_str(), NULL);

    if (parsedPort < 0 || parsedPort > 65535)
        std::cout << "Warning : invalid port number" << std::endl;
    this->_listen = parsedPort;
}

void Server::setServerName(std::string& input)
{
    this->serverName = input;
}

void Server::setRoot(std::string& input)
{
    this->root = input;
}

bool Server::setErrorPage(std::string const& sentence)
{
    std::stringstream sstream;
    std::vector<std::string> tokens;
    std::string token;
    std::vector<int> vecErrorCode;
    std::string errorPath;

    sstream << sentence;
    while (sstream >> token)
        tokens.push_back(token);

    if (tokens.size() < 2)
        return true;
    errorPath = tokens.back();
    tokens.pop_back();

    for (size_t i = 0; i < tokens.size(); i++)
        vecErrorCode.push_back(std::strtod(tokens[i].c_str(), NULL));
    this->errorPage.insert(std::pair<std::vector<int>, std::string>(vecErrorCode, errorPath));
    return false;
}

bool Server::setRedirection(std::string const& sentence)
{
    std::stringstream sstream;
    std::vector<std::string> tokens;
    std::string token;
    std::string redirectionPath;
    int redirectionCode;

    sstream << sentence;
    while (sstream >> token)
        tokens.push_back(token);

    if (tokens.size() != 2)
        return true;
    redirectionCode = std::strtod(tokens[0].c_str(), NULL);
    redirectionPath = tokens[1];
    this->redirection[redirectionPath] = redirectionCode;
    return false;
}

void Server::setAutoIndex(std::string& input)
{
    if (input == "on")
        this->autoIndex = true;
    else
        this->autoIndex = false;
}

void Server::setClientBodySize(std::string& input)
{
    this->clientBodySize = std::strtod(input.c_str(), NULL);
}

int Server::openPort()
{

    struct addrinfo* info;
    struct addrinfo hint;
    struct sockaddr_in socketaddr;
    int opt = 1;

    std::cout << "Port number : " << _listen << std::endl;

    memset(&hint, 0, sizeof(struct addrinfo));
    memset(&socketaddr, 0, sizeof(struct sockaddr_in));

    socketaddr.sin_family = AF_INET;
    socketaddr.sin_port = htons(_listen);
    socketaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    hint.ai_family = AF_INET;
    hint.ai_socktype = SOCK_STREAM;

    std::string strPortNumber = intToString(_listen);
    std::cout << "strPortNumber : " << strPortNumber << std::endl;

    int errorCode = getaddrinfo(serverName.c_str(), strPortNumber.c_str(), &hint, &info);
    if (errorCode == -1)
        exitWebserver(gai_strerror(errorCode));

    serverSocket = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
    std::cout << serverSocket << std::endl;
    if (serverSocket == -1)
        exitWebserver("socket() error");
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    errorCode = bind(serverSocket, reinterpret_cast<struct sockaddr*>(&socketaddr), sizeof(socketaddr));
    if (errorCode)
        exitWebserver("bind() error");
    errorCode = listen(serverSocket, LISTENCAPACITY);
    if (errorCode)
        exitWebserver("listen() error");
    return serverSocket;
}

int Server::acceptClient()
{
    std::cout << "in acceptClient :: serverSocket : " << serverSocket << std::endl;
    int newClientFd = accept(serverSocket, NULL, NULL);
    if (newClientFd == -1)
        exitWebserver("Server : accept() error");
    fcntl(newClientFd, F_SETFL, O_NONBLOCK, FD_CLOEXEC);

    clients[newClientFd] = Client();
    clients[newClientFd].setFd(newClientFd);
    clients[newClientFd].setLocations(locations);

    events.changeEvents(newClientFd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, this);
    events.changeEvents(newClientFd, EVFILT_WRITE, EV_ADD | EV_DISABLE, 0, 0, this);
    return newClientFd;
}

bool Server::isClient(int ident)
{
    return clients.find(ident) != clients.end();
}

Client& Server::getClient(int ident)
{
    return clients[ident];
}

void Server::disconnectClient(int ident)
{
    close(ident);
    clients.erase(ident);
}
