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

# define SPACE " "
# define CRLF "\r\n"

class	Client;

class Handler
{
	public:
		std::string	getFileType(std::string file_name) const;
		std::string	itos(int num);
		std::vector<unsigned char>	stou(std::stringstream& ss);
		// void		buildHeader(int status_code);
		bool		IsDirectory(std::string path);
		bool		IsRegularFile(std::string path);
		bool		IsFileReadble(std::string path);
		bool		IsFileExist(std::string path);
		virtual std::vector<unsigned char>	handle(Client& client) const = 0;
		virtual ~Handler();
};
