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

class MessageReader
{
public:
	static std::map<int, std::stringstream> readBuffer;
	static std::map<int, Message> messageBuffer;
	static std::map<int, enum RequestMessageParseState> ParseState;
	void readHeader(const char *buffer, int client_fd);
	void readBody(const char *buffer, int client_fd);
	~MessageReader() {};
private:
	MessageReader() {};
};