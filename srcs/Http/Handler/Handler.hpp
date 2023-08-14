#pragma once

#include "../HttpRequest.hpp"
#include <vector>
#include "../../Color.hpp" // for debug
#include <iostream> // for debug
#include <sstream> // for stringstream
#include <fstream> // for ifstream
#include <sys/stat.h> // for struct stat
#include <dirent.h> // for directory listing/entrying

# define SPACE " "
# define CRLF "\r\n"
# define SERVER_NAME "Master J&J Server"
# define SERVER_HTTP_VERSION "HTTP/1.1"
# define COLON ":"

class	Client;

class Handler
{
	private:
	public:
		static std::vector<unsigned char>	BuildHeader(int status_code, std::map<std::string, std::string>& headers, bool include_crlf=true);
		static std::string	GetFileType(std::string file_name);
		static std::string	itos(int num);
		static std::vector<unsigned char>	stou(std::stringstream& ss);
		static std::vector<unsigned char>	BuildResponse(int status_code, std::map<std::string, std::string>& headers, std::vector<unsigned char>& body);
		static std::vector<unsigned char>	ReadStaticFile(std::string& file_name);
		static bool		IsDirectory(std::string path);
		static bool		IsRegularFile(std::string path);
		static bool		IsFileReadable(std::string path);
		static bool		IsFileExist(std::string path);
		static std::vector<unsigned char>	ServeStatic(std::string& path);
		virtual std::vector<unsigned char>	handle(Client& client) const = 0;
		virtual ~Handler();
};
