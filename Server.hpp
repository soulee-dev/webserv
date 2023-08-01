#pragma once

#include "Location.hpp"
#include "RequestMessage.hpp"
#include "ResponseMessage.hpp"
#include <map>
#include <string>
class Server
{
private:
    int listen;
    std::string serverName;
    std::string root;
    std::map<std::vector<int>, std::string> errorPage;
    std::map<std::string, int> redirection;
    bool autoIndex;
    size_t clientBodySize;
    std::map<std::string, Location> locations;

    Server(Server const& other);
    Server& operator=(Server const& rhs);

    void setListen(std::string& input);
    void setServerName(std::string& input);
    void setRoot(std::string& input);
    bool setErrorPage(std::string const& sentence);
    bool setRedirection(std::string const& sentence);
    void setAutoIndex(std::string& input);
    void setClientBodySize(std::string& input);
public:
    Server();
    ~Server();
    bool fillServer(std::map<std::string, Location>& mapLocations, std::map<std::string, std::string>& mapSentence);
    int getListen() const;
    std::string getServerName() const;
    int runServer(RequestMessage& req, ResponseMessage& res);
    void DebugPrint();
};
