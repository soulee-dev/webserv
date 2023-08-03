#include "ConfigParser.hpp"
#include "ServerManager.hpp"
#include <iostream>


int main(int argc, char **argv)
{
	if (argc > 2)
	{
		std::cout << "please input one config file" << std::endl;
		exit(1);
	}

	ConfigParser &configParser = ConfigParser::getInstance();
	ServerManager &serverManager = ServerManager::getInstance();

	if (argc == 1)
	{
		const std::string	file = "default.conf";
		configParser.parseConfig(file);
		serverManager.setServers(configParser.server);
	}
	else
	{
		configParser.parseConfig(argv[1]);
		serverManager.setServers(configParser.server);
	}
	serverManager.initServers();
	serverManager.runServerManager();
}