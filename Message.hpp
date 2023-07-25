#pragma once
#include <string>
#include <vector>
#include <map>

class Message
{
public:
	Message() {};
	~Message() {};
	std::vector<unsigned char> raw;
	std::string startLine;
	std::map<std::string, std::string> headers;
};