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
	std::map<int, std::vector<unsigned char>> readBuffer;
	std::map<int, Message> messageBuffer;
	virtual void readStartLine(const char *buffer, Message &message) = 0;
	virtual void readHeader(const char *buffer, Message &message);
	virtual void readBody(const char *buffer, Message &message);
private:
};