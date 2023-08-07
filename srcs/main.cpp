#include "ConfigParser.hpp"
#include "ServerManager.hpp"
#include <iostream>

int main(int argc, char** argv)
{
    std::string configFileName;

    if (argc > 2)
    {
        std::cout << "please input only one file..." << std::endl;
        return 1;
    }
    if (argc == 0)
    {
        std::cout << "use default config file..." << std::endl;
        configFileName = "./config/default.conf";
    }
    else
        configFileName = argv[1];
    ConfigParser configParser;
    ServerManager serverManager;

    configParser.parseConfig(configFileName);
    serverManager.setServers(configParser.server);

    serverManager.initServers();
    serverManager.runServerManager();
}