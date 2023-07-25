#pragma once
#include "Message.hpp"
#include <sstream>

enum MessageParseState
{
	START,
	HEADER,
	BODY,
	DONE
};

class MessageReader
{
public:
	MessageReader() {};
	~MessageReader() {};
	std::map<int, std::stringstream> readBuffer;
	std::map<int, Message> messageBuffer;
	virtual void readStartLine(const char *buffer, int client_fd) = 0;
	virtual void readHeader(const char *buffer, int client_fd);
	virtual void readBody(const char *buffer, int client_fd);
private:
};