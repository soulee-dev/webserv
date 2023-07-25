#include "MessageReader.hpp"

void MessageReader::readHeader(const char *buffer, Message &message)
{
	std::stringstream sstream;
	std::stringstream headerSstream;
	std::string line;
	std::string key;
	std::string value;

	message.raw.insert(message.raw.end(), buffer, buffer + strlen(buffer));
	sstream << buffer;
	while (getline(sstream, line, '\n'))
	{
		if (*line.end() == '\r')
			line.pop_back();
		if (line.empty())
		{
			message.parseState = BODY;
			break;
		}
		headerSstream << line;
		getline(headerSstream, key, ':');
		getline(headerSstream, value);
		message.headers[key] = value;
	}
}

void MessageReader::readBody(const char *buffer, Message &message)
{
	std::stringstream sstream;
	std::string line;

	message.raw.insert(message.raw.end(), buffer, buffer + strlen(buffer));
	while (getline(sstream, line, '\n'))
	{
		if (line.empty())
		{
			message.parseState = DONE;
			return ;
		}
	}
}