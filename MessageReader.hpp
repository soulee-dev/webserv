#pragma once
#include "Message.hpp"
#include <sstream>

// 일단 버퍼에 담고 나서 현재 state에 있는 토큰을 읽어야 한다.

enum RequestMessageParseState
{
	METHOD,
	REQUEST_TARGET,
	HTTP_VERSION,
	HEADER,
	BODY,
	DONE
};

enum RespondMessageParseState
{
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