#pragma once
#include "MessageReader.hpp"

class RespondMessageReader
{
public:
	RespondMessageReader() {};
	~RespondMessageReader() {};
	void readStartLine(const char *buffer, Message &message);
};