#pragma once

#include "Server.hpp"
#include <iostream>
#include <map>
#include <sstream>
#include <vector>
#define CONFIG_ERROR -1

enum METHOD
{
    GET = 1,
    POST = 2,
    PUT = 4,
    DELETE = 8,
    HEAD = 16,
};

class Location;
class Server;
class ConfigParser
{
public:
    std::map<int, Server> server;
    void parseConfig(std::string const& configFileName);
    ~ConfigParser();
    ConfigParser();

private:
    typedef std::map<int, Server> mapPortServer;
    typedef std::map<std::string, Location> mapStrLocation;
    typedef std::vector<std::string> vecStr;
    typedef std::map<std::string, std::string> mapStrStr;

    ConfigParser(ConfigParser const& other);
    ConfigParser& operator=(ConfigParser const& rhs);

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
