#pragma once
#include "RequestMessage.hpp"

enum RequestMessageParseState
{
	METHOD,
	REQUEST_TARGET,
	HTTP_VERSION,
	HEADER,
	BODY,
	DONE
};

class RequestMessageReader
{
public:
	static RequestMessageReader &getInstance()
	{
		static RequestMessageReader instance;
		return instance;
	}
	~RequestMessageReader() {};
	std::map<int, RequestMessage> messageBuffer;
	std::map<int, std::vector<unsigned char> > readBuffer;
	std::map<int, enum RequestMessageParseState> ParseState;
	void readMethod(const char *buffer, int client_fd);
	void readRequestTarget(const char *buffer, int client_fd);
	void readHttpVersion(const char *buffer, int client_fd);
	void readHeader(const char *buffer, int client_fd);
	void readBody(const char *buffer, int client_fd);
private:
	RequestMessageReader() {};
};