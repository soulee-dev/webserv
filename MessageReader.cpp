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
			ParseState[client_fd] = RequestMessageParseState::BODY;
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
	Message &currMessage = messageBuffer[client_fd];
	std::stringstream &currReadBuffer = readBuffer[client_fd];
	size_t pos;

	readBuffer[client_fd] << buffer;
	if ((pos = readBuffer[client_fd].str().find("\r\n\r\n")) != std::string::npos)
	{
		currMessage.body = std::vector<unsigned char>(readBuffer[client_fd].str().begin(), readBuffer[client_fd].str().begin() + pos);
		currReadBuffer.str(currReadBuffer.str().substr(pos + 4));
		ParseState[client_fd] = RequestMessageParseState::DONE;
		return ;
	}
	;
}