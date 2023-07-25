#pragma once
#include "MessageReader.hpp"

class RequestMessageReader : public MessageReader
{
public:
	RequestMessageReader() {};
	~RequestMessageReader() {};
	void readStartLine(const char *buffer, Message &message);
};