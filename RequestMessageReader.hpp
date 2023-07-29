#pragma once
#include "RequestMessage.hpp"

enum RequestMessageParseState
{
	METHOD,
	REQUEST_TARGET,
	HTTP_VERSION,
	HEADER,
	BODY,
	DONE,
	ERROR
};

class RequestMessageReader
{
public:
	static RequestMessageReader &getInstance();
	~RequestMessageReader() {};
	std::map<int, RequestMessage> messageBuffer;
	std::map<int, std::vector<unsigned char> > readBuffer;
	std::map<int, enum RequestMessageParseState> ParseState;
    bool readMessage(int client_fd);

	void insertNewClient(int client_fd);
	void deleteClient(int client_fd);
private:
	void readMethod(const char *buffer, int client_fd);
	void readRequestTarget(const char *buffer, int client_fd);
	void readHttpVersion(const char *buffer, int client_fd);
	void readHeader(const char *buffer, int client_fd);
	void readBody(const char *buffer, int client_fd);
	RequestMessageReader() {};
};