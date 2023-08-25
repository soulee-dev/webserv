#include "Utils.hpp"
#include <iostream>
#include <map>
#include <sstream>
#include <fstream>
#include <vector>
#include <sys/stat.h>

std::string intToString(int num)
{
    std::stringstream sstream(num);
    return sstream.str();
}

void exitWebserver(std::string msg)
{
    std::cout << msg << std::endl;
    exit(1);
}

size_t minLen(size_t a, size_t b)
{
    if (a > b)
        return b;
    else
        return a;
}

std::string	getFileType(std::string file_name)
{
	std::string	file_type;

	if (file_name.find(".html") != std::string::npos || file_name.find(".htm") != std::string::npos)
		file_type = "text/html";
	else if (file_name.find(".gif") != std::string::npos)
		file_type = "image/gif";
	else if (file_name.find(".png") != std::string::npos)
		file_type = "image/png";
	else if (file_name.find(".jpg") != std::string::npos)
		file_type = "image/jpeg";
	else if (file_name.find(".mpg") != std::string::npos)
		file_type = "video/mpg";
	else if (file_name.find(".mp4") != std::string::npos)
		file_type = "video/mp4";
	else
		file_type = "text/plain";
	return (file_type);
}


std::vector<unsigned char>	stou(std::stringstream& ss)
{
	std::string					tmp_string = ss.str();
	std::vector<unsigned char>	result(tmp_string.begin(), tmp_string.end());
	return result;
}


bool	isDirectory(std::string path)
{
	struct stat	buf;


	if (stat(path.c_str(), &buf) == 0)
	{
		if (S_ISDIR(buf.st_mode))
			return true;
	}
	return false;
}

bool isRegularFile(std::string path)
{	
	struct stat	buf;

	if (stat(path.c_str(), &buf) == 0)
	{
		if (S_ISREG(buf.st_mode))
			return true;
	}
	return false;
}

// Check permission file is readable
bool	isFileReadable(std::string path)
{	
	struct stat	buf;

	if (stat(path.c_str(), &buf) == 0)
	{
		if (S_IRUSR & buf.st_mode)
			return true;
	}
	return false;
}

bool	isFileExist(std::string path)
{	
	struct stat	buf;

	if (stat(path.c_str(), &buf) == 0)
		return true;
	return false;
}

std::vector<unsigned char>	ReadStaticFile(std::string& file_name)
{
	std::ifstream	file(file_name.c_str(), std::ios::in | std::ios::binary);

	file.seekg(0, std::ios::end);
	int length = file.tellg();
	file.seekg(0, std::ios::beg);

	std::vector<unsigned char> buffer(length);
	file.read(reinterpret_cast<char*>(&buffer[0]), length);
	return buffer;
}
