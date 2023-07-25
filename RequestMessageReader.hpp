#pragma once
#include "MessageReader.hpp"

class RequestMessageReader : public MessageReader
{
public:
	~RequestMessageReader() {};
	void readMethod(const char *buffer, int client_fd);
	void readRequestTarget(const char *buffer, int client_fd);
	void readHttpVersion(const char *buffer, int client_fd);
};