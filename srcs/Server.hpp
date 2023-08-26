#include "Client.hpp"
#include "Event.hpp"
#include "Location.hpp"
#include <fcntl.h>
#include <map>
#include <netdb.h>
#include <string>

extern Event events;
class Server
{
private:
    int _listen;
    std::string serverName;
    std::string root;
    // struct addrinfo* info;
    // struct addrinfo hint;
    // struct sockaddr_in socketaddr;
    std::map<std::vector<int>, std::string> errorPage;
    std::map<std::string, int> redirection;
    bool autoIndex;
    size_t clientBodySize;
    std::map<std::string, Location> locations;
    std::map<int, Client> clients;

    // setter
    void setListen(std::string& input);
    void setServerName(std::string& input);
    void setRoot(std::string& input);
    bool setErrorPage(std::string const& sentence);
    bool setRedirection(std::string const& sentence);
    void setAutoIndex(std::string& input);
    void setClientBodySize(std::string& input);

public:
    static const int LISTENCAPACITY = 1000;
    int serverSocket;
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
    int openPort(void);
    int acceptClient();
    bool isClient(int ident);
    Client& getClient(int ident);
    void disconnectClient(int ident);

    int readProcessFromClient(int ident);
	void staticRequestProcess(int ident);
};
