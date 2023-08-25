#pragma once
#include <string>

std::string intToString(int number);
void exitWebserver(std::string msg);
size_t minLen(size_t a, size_t b);
std::string	getFileType(std::string file_name);
std::vector<unsigned char>	stou(std::stringstream& ss);
bool isDirectory(std::string path);
bool isRegularFile(std::string path);
bool isFileReadable(std::string path);
bool isFileExist(std::string path);
std::vector<unsigned char> ReadStaticFile(std::string& file_name);
