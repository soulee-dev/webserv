#include "../includes/Server.hpp"
#include "../includes/csapp.h"

int main(int argc, char **argv)
{
	struct addrinfo *info;
	struct addrinfo hint;
	struct sockaddr_in socketaddr;
	int errorcode;
	int server_socket;

	if (argc > 2)
	{
		std::cout << "please input config file" << std::endl;
		exit(1);
	}

	Server tiny("0.0.0.0", 8080);

	tiny.run();
	return 0;
}

// config file 파싱, 서버 블록 여러 개 들어올 때 서버가 여러 개 열리도록 처리해야 함