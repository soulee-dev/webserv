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
	MessageReader() {};
	~MessageReader() {};
	virtual void readStartLine(const char *buffer, Message &message) = 0;
	virtual void readHeader(const char *buffer, Message &message);
	virtual void readBody(const char *buffer, Message &message);
private:
};