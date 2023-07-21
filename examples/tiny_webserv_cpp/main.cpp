#include "Server.hpp"

int	main()
{
	Server	server = Server("0.0.0.0", 80);
	server.run();

	return (0);
}