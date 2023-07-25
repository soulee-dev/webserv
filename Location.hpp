#pragma once
#include <map>
#include <string>
#include <vector>

class Server;
class Location
{
public:
    Location() : root("/"), allowMethod(0), autoIndex(false), clientBodySize(10240) {};
    ~Location() {};
    bool fillLocationBlock(std::map<std::string, std::string> &mapSentence);
    void DebugPrint();
private:
    std::string root;
    std::map<std::vector<int>, std::string> errorPage;
    std::map<std::string, int>redirection;
    unsigned int allowMethod;
    std::vector<std::string> index;
    bool autoIndex;
    size_t clientBodySize;

    void setRoot(std::string &input);
    bool setErrorPage(std::string const &sentence);
    bool setRedirection(std::string const &sentence);
    void setAutoIndex(std::string &input);
    void setClientBodySize(std::string &input);
    bool setAllowMethod(std::string &sentence);
    void setIndex(std::string &sentence);
};
