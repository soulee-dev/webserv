#pragma once

#include "../HttpRequest.hpp"
#include <vector>
// --- //
#include "../../Color.hpp" // for debug
#include <iostream> // for debug
#include <sstream> // for stringstream
#include <fstream> // for ifstream
#include <sys/stat.h> // for struct stat
#include <dirent.h> // for directory listing/entrying
// --- //

class Handler
{
	public:
		virtual std::vector<unsigned char>	handle(HttpRequest& request) const = 0;
};