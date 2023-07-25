#pragma once
#include <map>
#include "Location.hpp"

class Server
{
public:
    Server() : listen(80), serverName(""), root("/"), autoIndex(false), clientBodySize(10240) {};
    ~Server() {};
    bool fillServer(std::map<std::string, Location> &mapLocations, std::map<std::string, std::string> &mapSentence);
    int getListen() const { return this->listen; };
    std::string getServerName() const { return this->serverName; };
    void DebugPrint();
private:
    int listen;
    std::string serverName;
    std::string root;
    std::map<std::vector<int>, std::string> errorPage;
    std::map<std::string, int>redirection;
    bool autoIndex;
    size_t clientBodySize;
    std::map<std::string, Location> locations;

    void setListen(std::string &input);
    void setServerName(std::string &input);
    void setRoot(std::string &input);
    bool setErrorPage(std::string const &sentence);
    bool setRedirection(std::string const &sentence);
    void setAutoIndex(std::string &input);
    void setClientBodySize(std::string &input);
};