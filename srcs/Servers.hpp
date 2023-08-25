#pragma once
#include "Event.hpp"
#include "Server.hpp"
#include <iostream>
#include <map>
#include <sstream>
#include <vector>
#define CONFIG_ERROR -1

extern Event events;
class Location;
class Server;
class Servers
{
public:
    ~Servers();
    Servers();
    std::map<int, Server> server;
    std::map<int, int> portByServerSocket;
    void parseConfig(std::string const& configFileName);
    void initServers(void);
    bool isResponseToServer(int ident);
    void serverDisconnect(int ident);

private:
    typedef std::map<int, Server> mapPortServer;
    typedef std::map<std::string, Location> mapStrLocation;
    typedef std::vector<std::string> vecStr;
    typedef std::map<std::string, std::string> mapStrStr;

    Servers(Servers const& other);
    Servers& operator=(Servers const& rhs);

    struct s_info
    {
        std::string inputToken;
        std::map<std::string, Location> locations;
        std::string locationDir;
        std::vector<std::string> vecInput;
        std::map<std::string, std::string> mapSentence;
    };

    static int parse_action_0(struct s_info& parse_info, mapPortServer& servers);
    static int parse_action_1(struct s_info& parse_info, mapPortServer& servers);
    static int parse_action_2(struct s_info& parse_info, mapPortServer& servers);
    static int parse_action_3(struct s_info& parse_info, mapPortServer& servers);
    static int parse_action_4(struct s_info& parse_info, mapPortServer& servers);
    static int parse_action_5(struct s_info& parse_info, mapPortServer& servers);
    static int parse_action_6(struct s_info& parse_info, mapPortServer& servers);
    static int parse_action_7(struct s_info& parse_info, mapPortServer& servers);
};
