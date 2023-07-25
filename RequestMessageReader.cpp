#include "RequestMessageReader.hpp"
#include "RequestMessage.hpp"

void RequestMessageReader::readMethod(const char *buffer, int client_fd)
{
	std::string method;
	RequestMessage &currMessage = static_cast<RequestMessage&>(messageBuffer[client_fd]);
	size_t pos;

	readBuffer[client_fd] << buffer;
	if ((pos = readBuffer[client_fd].str().find(' ')) == std::string::npos)
		return ;
	else
	{
		method = readBuffer[client_fd].str().substr(0, pos);
		currMessage.startLine = method;
		readBuffer[client_fd].str(readBuffer[client_fd].str().substr(pos + 1));
		ParseState[client_fd] = RequestMessageParseState::REQUEST_TARGET;
		if (pos = readBuffer[client_fd].str().find(' ') != std::string::npos)
			readRequestTarget(NULL, client_fd);
		return ;
	}
}

void RequestMessageReader::readRequestTarget(const char *buffer, int client_fd)
{
	std::string RequestTarget;
	RequestMessage &currMessage = static_cast<RequestMessage&>(messageBuffer[client_fd]);
	size_t pos;

	readBuffer[client_fd] << buffer;
	if ((pos = readBuffer[client_fd].str().find(' ')) == std::string::npos)
		return ;
	else
	{
		RequestTarget = readBuffer[client_fd].str().substr(0, pos);
		currMessage.startLine = RequestTarget;
		readBuffer[client_fd].str(readBuffer[client_fd].str().substr(pos + 1));
		ParseState[client_fd] = RequestMessageParseState::HTTP_VERSION;
		if (pos = readBuffer[client_fd].str().find(' ') != std::string::npos)
			readHttpVersion(NULL, client_fd);
		return ;
	}
}

void RequestMessageReader::readHttpVersion(const char *buffer, int client_fd)
{
	std::string httpVersion;
	RequestMessage &currMessage = static_cast<RequestMessage&>(messageBuffer[client_fd]);
	size_t pos;

	readBuffer[client_fd] << buffer;
	if ((pos = readBuffer[client_fd].str().find(' ')) == std::string::npos)
		return ;
	else
	{
		httpVersion = readBuffer[client_fd].str().substr(0, pos);
		currMessage.startLine = httpVersion;
		readBuffer[client_fd].str(readBuffer[client_fd].str().substr(pos + 1));
		ParseState[client_fd] = RequestMessageParseState::HEADER;
		if (pos = readBuffer[client_fd].str().find('\n') != std::string::npos)
			readHeader(NULL, client_fd);
		return ;
	}
}