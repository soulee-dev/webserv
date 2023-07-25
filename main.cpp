#include "config.hpp"
#include "RequestMessage.hpp"
#include "RespondMessage.hpp"
#include "ServerManager.hpp"
#include <iostream>

int main(int argc, char **argv)
{
    struct addrinfo *info;
    struct addrinfo hint;
    struct sockaddr_in socketaddr;
    int errorcode;
    int server_socket;

    if (argc != 2)
    {
        std::cout << "please input one config file" << std::endl;
        exit(1);
    }
    ConfigParser configParser = ConfigParser::getInstance();
    configParser.parseConfig(argv[1]);

    ServerManager serverManager = ServerManager::getInstance();
    serverManager.servers = configParser.server;

    serverManager.init_server();
	std::cout << "echo server started" << std::endl;
    serverManager.start_server();

    return 0;
}