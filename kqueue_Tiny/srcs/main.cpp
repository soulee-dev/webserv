#include "../includes/ServerManager.hpp"
#include "../includes/csapp.h"
#include "../includes/Config.hpp"
#include "../includes/Color.hpp"

int main(int argc, char **argv)
{
	struct addrinfo	*info;
	struct addrinfo hint;
	struct sockaddr_in socketaddr;
	int errorcode;
	int server_socket;

	if (argc > 2)
	{
		std::cout << BOLDRED << "Please input Config file" << RESET << std::endl;
		exit(1);
	}

	ConfigParser configParser = ConfigParser::getInstance();
	std::string const conf = "default.conf";

	if (argc == 1)
		configParser.parseConfig(conf);
	else
		configParser.parseConfig(argv[1]);

	ServerManager tiny("0.0.0.0", 8080);

	tiny.run();
	return 0;
}

// config file 파싱