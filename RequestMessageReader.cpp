#include "RequestMessageReader.hpp"
#include <sstream>
#include <algorithm>

void RequestMessageReader::readHeader(const char *buffer, int client_fd)
{
	std::stringstream headerSstream;
	std::string line;
	std::string key;
	std::string value;
	RequestMessage &currMessage = messageBuffer[client_fd];
	std::vector<unsigned char> &currReadBuffer = readBuffer[client_fd];
	std::vector<unsigned char>::iterator pos;

	currReadBuffer.insert(currReadBuffer.end(), buffer, buffer + strlen(buffer));

	while ((pos = std::search(currReadBuffer.begin(), currReadBuffer.end(), "\n", &"\n"[1])) != currReadBuffer.end())
	{
		line = std::string(currReadBuffer.begin(), pos);
		currReadBuffer.erase(currReadBuffer.begin(), pos + 1);
		if (line[line.size() - 1] == '\r')
			line.pop_back();
		if (line.empty())
		{
			ParseState[client_fd] = BODY;
			return (readBody("", client_fd));
		}
		headerSstream << line;
		getline(headerSstream, key, ':');
		getline(headerSstream, value);
		headerSstream.clear();
		// value의 처음 ifs들을 지움 근데 하나가 아닐 수 있음
		while (value[0] == ' ')
			value.erase(value.begin());
		for (int i = 0; i < key.size(); i++)
			key[i] = tolower(key[i]);
		currMessage.headers[key] = value;
	}
}

void RequestMessageReader::readBody(const char *buffer, int client_fd)
{
	RequestMessage &currMessage = messageBuffer[client_fd];
	std::vector<unsigned char> &currReadBuffer = readBuffer[client_fd];
	std::vector<unsigned char>::iterator pos;

	currReadBuffer.insert(currReadBuffer.end(), buffer, buffer + strlen(buffer));
	if (std::search(currReadBuffer.begin(), currReadBuffer.end(), "\r\n\r\n", &"\r\n\r\n"[4]) != currReadBuffer.end())
	{
		currMessage.body = std::vector<unsigned char>(currReadBuffer.begin(), pos);
		currReadBuffer.erase(currReadBuffer.begin(), pos + 4);
		ParseState[client_fd] = DONE;
		return ;
	}
	else if (currReadBuffer.size() == 0)
	{
		// 사실 이때 매서드와 헤더를 보고 에러인지 판단해야함.
		ParseState[client_fd] = DONE;
		return ;
	}
}
void RequestMessageReader::readMethod(const char *buffer, int client_fd)
{
	std::string method;
	RequestMessage &currMessage = static_cast<RequestMessage&>(messageBuffer[client_fd]);
	std::vector<unsigned char> &currReadBuffer = readBuffer[client_fd];
	std::vector<unsigned char>::iterator pos;

	if (strcmp(buffer, "\n") == 0 | strcmp(buffer, "\r\n") == 0)
		return ;
	currReadBuffer.insert(currReadBuffer.end(), buffer, buffer + strlen(buffer));
	if ((pos = std::search(currReadBuffer.begin(), currReadBuffer.end(), " ", &" "[1])) != currReadBuffer.end())
	{
		method = std::string(currReadBuffer.begin(), pos);
		currMessage.startLine = method;
		currReadBuffer.erase(currReadBuffer.begin(), pos + 1);
		ParseState[client_fd] = REQUEST_TARGET;
		if ((pos = std::search(currReadBuffer.begin(), currReadBuffer.end(), " ", &" "[1])) != currReadBuffer.end())
			readRequestTarget("", client_fd);
		return ;
	}
}

void RequestMessageReader::readRequestTarget(const char *buffer, int client_fd)
{
	std::string RequestTarget;
	RequestMessage &currMessage = static_cast<RequestMessage&>(messageBuffer[client_fd]);
	std::vector<unsigned char> &currReadBuffer = readBuffer[client_fd];
	std::vector<unsigned char>::iterator pos;


	currReadBuffer.insert(currReadBuffer.end(), buffer, buffer + strlen(buffer));
	if ((pos = std::search(currReadBuffer.begin(), currReadBuffer.end(), " ", &" "[1])) != currReadBuffer.end())
	{
		RequestTarget = std::string(currReadBuffer.begin(), pos);
		currMessage.startLine += " " + RequestTarget;
		currReadBuffer.erase(currReadBuffer.begin(), pos + 1);
		ParseState[client_fd] = HTTP_VERSION;
		if ((pos = std::search(currReadBuffer.begin(), currReadBuffer.end(), " ", &" "[1])) != currReadBuffer.end())
			readHttpVersion("", client_fd);
		return ;
	}
}

void RequestMessageReader::readHttpVersion(const char *buffer, int client_fd)
{
	std::string httpVersion;
	RequestMessage &currMessage = static_cast<RequestMessage&>(messageBuffer[client_fd]);
	std::vector<unsigned char> &currReadBuffer = readBuffer[client_fd];
	std::vector<unsigned char>::iterator pos;


	currReadBuffer.insert(currReadBuffer.end(), buffer, buffer + strlen(buffer));
	if ((pos = std::search(currReadBuffer.begin(), currReadBuffer.end(), "\r\n", &"\r\n"[2])) != currReadBuffer.end())
	{
		httpVersion = std::string(currReadBuffer.begin(), pos);
		currMessage.startLine += " " + httpVersion;
		currReadBuffer.erase(currReadBuffer.begin(), pos + 2);
		ParseState[client_fd] = HEADER;
		if ((pos = std::search(currReadBuffer.begin(), currReadBuffer.end(), "\r\n", &"\r\n"[2])) != currReadBuffer.end())
			readHeader("", client_fd);
		return ;
	}
}