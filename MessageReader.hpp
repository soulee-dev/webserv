#pragma once
#include "Message.hpp"
#include <sstream>

enum RequestMessageParseState
{
	START,
	METHOD,
	REQUEST_TARGET,
	HTTP_VERSION,
	HEADER,
	BODY,
	DONE
};

enum RespondMessageParseState
{
	START,
	HTTP_VERSION,
	STATUS_CODE,
	REASON_PHRASE,
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