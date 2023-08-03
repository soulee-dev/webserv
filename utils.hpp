#pragma once

#include <iostream>
#include <sstream>
#include <string>

#include "ServerManager.hpp"

namespace utils
{
	std::string	build_header(std::string status_code, int file_size, std::string file_type);
	
}
