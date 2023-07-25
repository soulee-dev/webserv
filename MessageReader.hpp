#pragma once

class MessageReader
{
public:
	MessageReader();
	~MessageReader();
	int state;
	enum State
	{
		START,
		HEADER,
		BODY,
		DONE
	};
	static void readStartLine(const char *buffer);
	static void readHeader(const char *buffer);
	static void readBody(const char *buffer);
};