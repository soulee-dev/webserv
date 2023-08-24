#pragma once

#include "Handler.hpp"

void	HandleStatic(Client& client);
void	ProcessDirectory(Client& client);
void	HandleDirectoryListing(Client& client, Request& request);
int	is_directory(std::string fileName);

