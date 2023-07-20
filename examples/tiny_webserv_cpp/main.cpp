#include <string>
#include <iostream>
#include "Server.hpp"

int	main(int argc, char **argv)
{
	Server	serv("0.0.0.0", 8080);
	serv.Listen();

	return 0;
}