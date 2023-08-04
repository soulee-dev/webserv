#include "ConfigParser.hpp"
#include "ServerManager.hpp"
#include <iostream>
#include <fstream>
#include "Color.hpp"

int main(int argc, char** argv)
{
	if (argc > 2)
	{
		std::cout << "please input one config file" << std::endl;
		exit(1);
	}
	ConfigParser configParser;
	ServerManager serverManager;

	if (argc == 1)
	{
		const std::string	file = "srcs/default.conf";
		std::ifstream	def(file);
		if (def.is_open())
		{
			configParser.parseConfig(file);
			serverManager.setServers(configParser.server);
		}
		else
		{
			std::cerr << BOLDRED << "Failed to open Default Configuration file" << std::endl;
			return 0;
		}
	}
	
	else
	{
		configParser.parseConfig(argv[1]);
		serverManager.setServers(configParser.server);
	}
	
	serverManager.initServers();
	serverManager.runServerManager();
}