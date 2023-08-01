#include "ConfigParser.hpp"
#include "ServerManager.hpp"
#include <iostream>


int main(int argc, char **argv)
{
	if (argc != 2)
	{
		std::cout << "please input one config file" << std::endl;
		exit(1);
	}
	ConfigParser configParser;
	ServerManager serverManager;

	configParser.parseConfig(argv[1]);
	serverManager.setServers(configParser.server);

	serverManager.initServers();
	serverManager.runServerManager();
}