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
			if (currMessage.headers.find("host") == currMessage.headers.end())
			{
				ParseState[client_fd] = ERROR;
				return ;
			}
			else
			{
				ParseState[client_fd] = BODY;
				return (readBody("", client_fd));
			}
		}
		headerSstream << line;
		getline(headerSstream, key, ':');
		if (key.size() == 0)
		{
			ParseState[client_fd] = ERROR;
			return ;
		}
		for (int i = 0; i < key.size(); i++)
		{
			if (isspace(key[i]))
			{
				ParseState[client_fd] = ERROR;
				return ;
			}
		}
		getline(headerSstream, value);
		if (value.size() == 0)
		{
			ParseState[client_fd] = ERROR;
			return ;
		}
		headerSstream.clear();
		// value의 처음 ifs들을 지움 근데 하나가 아닐 수 있음
		while (value[0] == ' ')
			value.erase(value.begin());
		for (int i = 0; i < key.size(); i++)
			key[i] = tolower(key[i]);
		currMessage.headers[key] = value;
	}
}

#include <iostream>

void RequestMessageReader::readBody(const char *buffer, int client_fd)
{
	RequestMessage &currMessage = messageBuffer[client_fd];
	std::vector<unsigned char> &currReadBuffer = readBuffer[client_fd];
	std::vector<unsigned char>::iterator pos;

	if (currMessage.method == "GET" || currMessage.method == "DELETE")
	{
		ParseState[client_fd] = DONE;
		return ;
	}
	currReadBuffer.insert(currReadBuffer.end(), buffer, buffer + strlen(buffer));
	std::string a;
	if ((pos = std::search(currReadBuffer.begin(), currReadBuffer.end(), "\r\n\r\n", &"\r\n\r\n"[4])) != currReadBuffer.end())
	{
		currMessage.body = std::vector<unsigned char>(currReadBuffer.begin(), pos);
		currReadBuffer.erase(currReadBuffer.begin(), pos + 4);
		ParseState[client_fd] = DONE;
	}
}
void RequestMessageReader::readMethod(const char *buffer, int client_fd)
{
	RequestMessage &currMessage = static_cast<RequestMessage&>(messageBuffer[client_fd]);
	std::vector<unsigned char> &currReadBuffer = readBuffer[client_fd];
	std::vector<unsigned char>::iterator pos;

	if (strcmp(buffer, "\n") == 0 | strcmp(buffer, "\r\n") == 0)
		return ;
	currReadBuffer.insert(currReadBuffer.end(), buffer, buffer + strlen(buffer));
	if ((pos = std::search(currReadBuffer.begin(), currReadBuffer.end(), " ", &" "[1])) != currReadBuffer.end())
	{
		currMessage.method = std::string(currReadBuffer.begin(), pos);
		currMessage.startLine = currMessage.method;
		currReadBuffer.erase(currReadBuffer.begin(), pos + 1);
		ParseState[client_fd] = REQUEST_TARGET;
		if ((pos = std::search(currReadBuffer.begin(), currReadBuffer.end(), " ", &" "[1])) != currReadBuffer.end())
			readRequestTarget("", client_fd);
		else if ((pos = std::search(currReadBuffer.begin(), currReadBuffer.end(), "\r\n", &"\r\n"[2])) != currReadBuffer.end())
		{
			ParseState[client_fd] = ERROR;
		}
	}
	else if ((pos = std::search(currReadBuffer.begin(), currReadBuffer.end(), "\r\n", &"\r\n"[2])) != currReadBuffer.end())
	{
		ParseState[client_fd] = ERROR;
		currReadBuffer.erase(currReadBuffer.begin(), pos + 2);
	}
}

void RequestMessageReader::readRequestTarget(const char *buffer, int client_fd)
{
	RequestMessage &currMessage = static_cast<RequestMessage&>(messageBuffer[client_fd]);
	std::vector<unsigned char> &currReadBuffer = readBuffer[client_fd];
	std::vector<unsigned char>::iterator pos;


	currReadBuffer.insert(currReadBuffer.end(), buffer, buffer + strlen(buffer));
	if ((pos = std::search(currReadBuffer.begin(), currReadBuffer.end(), " ", &" "[1])) != currReadBuffer.end())
	{
		currMessage.requestTarget = std::string(currReadBuffer.begin(), pos);
		currMessage.startLine += " " + currMessage.requestTarget;
		currReadBuffer.erase(currReadBuffer.begin(), pos + 1);
		ParseState[client_fd] = HTTP_VERSION;
		if ((pos = std::search(currReadBuffer.begin(), currReadBuffer.end(), "\r\n", &"\r\n"[2])) != currReadBuffer.end())
			readHttpVersion("", client_fd);
		else if ((pos = std::search(currReadBuffer.begin(), currReadBuffer.end(), " ", &" "[1])) != currReadBuffer.end())
		{
			ParseState[client_fd] = ERROR;
		}
	}
	else if ((pos = std::search(currReadBuffer.begin(), currReadBuffer.end(), "\r\n", &"\r\n"[2])) != currReadBuffer.end())
	{
		ParseState[client_fd] = ERROR;
		currReadBuffer.erase(currReadBuffer.begin(), pos + 2);
	}
}

void RequestMessageReader::readHttpVersion(const char *buffer, int client_fd)
{
	RequestMessage &currMessage = static_cast<RequestMessage&>(messageBuffer[client_fd]);
	std::vector<unsigned char> &currReadBuffer = readBuffer[client_fd];
	std::vector<unsigned char>::iterator pos;


	currReadBuffer.insert(currReadBuffer.end(), buffer, buffer + strlen(buffer));
	if ((pos = std::search(currReadBuffer.begin(), currReadBuffer.end(), "\r\n", &"\r\n"[2])) != currReadBuffer.end())
	{
		currMessage.httpVersion = std::string(currReadBuffer.begin(), pos);
		currReadBuffer.erase(currReadBuffer.begin(), pos + 2);
		if (currMessage.httpVersion != "HTTP/1.1" && currMessage.httpVersion != "HTTP/1.0")
		{
			ParseState[client_fd] = ERROR;
			return ;
		}
		else if (currMessage.method != "GET" && currMessage.method != "POST" && currMessage.method != "PUT" && currMessage.method != "DELETE")
		{
			ParseState[client_fd] = ERROR;
			return ;
		}
		currMessage.startLine += " " + currMessage.httpVersion;
		ParseState[client_fd] = HEADER;
		if ((pos = std::search(currReadBuffer.begin(), currReadBuffer.end(), "\r\n", &"\r\n"[2])) != currReadBuffer.end())
			readHeader("", client_fd);
		return ;
	}
}