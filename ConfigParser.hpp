#pragma once
#include "Server.hpp"
#include <iostream>
#include <map>
#include <sstream>
#include <vector>
# define CONFIG_ERROR -1
enum METHOD
{
    GET = 0,
    POST = 1,
    PUT = 2,
    DELETE = 4
};

class Location;
class Server;
class ConfigParser
{
public:
    static ConfigParser& getInstance()
    {
        static ConfigParser instance;
        return instance;
    }
    std::map<int, std::vector<Server> > server;
    void parseConfig(std::string const& configFileName);
    void DebugPrint();
    ~ConfigParser();

private:
    ConfigParser();
    typedef std::string str;
    typedef std::map<int, std::vector<Server> > mapPortServer;
    typedef std::map<std::string, Location> mapStrLocation;
    typedef std::vector<std::string> vecStr;
    typedef std::map<std::string, std::string> mapStrStr;

    static int parse_action_0(str& input, mapPortServer& servers, mapStrLocation& locations, str& LocationDir, vecStr& stack, mapStrStr& sentence);
    static int parse_action_1(str& input, mapPortServer& servers, mapStrLocation& locations, str& LocationDir, vecStr& stack, mapStrStr& sentence);
    static int parse_action_2(str& input, mapPortServer& servers, mapStrLocation& locations, str& LocationDir, vecStr& stack, mapStrStr& sentence);
    static int parse_action_3(str& input, mapPortServer& servers, mapStrLocation& locations, str& LocationDir, vecStr& stack, mapStrStr& sentence);
    static int parse_action_4(str& input, mapPortServer& servers, mapStrLocation& locations, str& LocationDir, vecStr& stack, mapStrStr& sentence);
    static int parse_action_5(str& input, mapPortServer& servers, mapStrLocation& locations, str& LocationDir, vecStr& stack, mapStrStr& sentence);
    static int parse_action_6(str& input, mapPortServer& servers, mapStrLocation& locations, str& LocationDir, vecStr& stack, mapStrStr& sentence);
    static int parse_action_7(str& input, mapPortServer& servers, mapStrLocation& locations, str& LocationDir, vecStr& stack, mapStrStr& sentence);
};
