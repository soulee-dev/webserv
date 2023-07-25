#pragma once

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
	MessageReader();
	~MessageReader();
	static void readStartLine(const char *buffer);
	static void readHeader(const char *buffer);
	static void readBody(const char *buffer);
};