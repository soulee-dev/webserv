#pragma once

#include "Handler.hpp"
#include <unistd.h>

bool OpenFd(Client& client);
bool RunCgi(Client& client);
