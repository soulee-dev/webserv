#include "MessageReader.hpp"

void MessageReader::readHeader(const char *buffer, int client_fd)
{
	std::stringstream sstream;
	std::stringstream headerSstream;
	std::string line;
	std::string key;
	std::string value;
	Message currMessage = messageBuffer[client_fd];

	readBuffer[client_fd] << buffer;
	while (getline(sstream, line, '\n'))
	{
		if (*line.end() == '\r')
			line.pop_back();
		if (line.empty())
		{
			currMessage.parseState = BODY;
			break;
		}
		headerSstream << line;
		getline(headerSstream, key, ':');
		getline(headerSstream, value);
		currMessage.headers[key] = value;
	}
}

void MessageReader::readBody(const char *buffer, int client_fd)
{
	std::stringstream sstream;
	std::string line;
	Message currMessage = messageBuffer[client_fd];

	readBuffer[client_fd] << buffer;
	while (getline(sstream, line, '\n'))
	{
		if (line.empty())
		{
			currMessage.body = std::vector<unsigned char>(readBuffer[client_fd].str().begin(), readBuffer[client_fd].str().end());
			currMessage.parseState = DONE;
			return ;
		}
	}
}