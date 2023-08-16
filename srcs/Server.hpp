#pragma once

#include "Location.hpp"
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

    // setter
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
    Server(Server const& other);
    Server& operator=(Server const& rhs);
    bool fillServer(std::map<std::string, Location>& mapLocations, std::map<std::string, std::string>& mapSentence);

    // getter
    int getListen() const;
    std::string getRoot() const;
    std::map<std::vector<int>, std::string> getErrorPage() const;
    std::map<std::string, int> getRedirection() const;
    bool getAutoIndex() const;
    unsigned int getClientBodySize() const;
    std::map<std::string, Location> getLocations() const;
    std::string getServerName() const;

    // function
};
