#pragma once

#include "Handler.hpp"
#include <unistd.h>

void OpenFd(Client& client);
void RunCgi(Client& client);
