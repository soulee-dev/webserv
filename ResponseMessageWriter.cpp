#include "ResponseMessageWriter.hpp"

ResponseMessageWriter::ResponseMessageWriter()
{
}

ResponseMessageWriter::~ResponseMessageWriter()
{
}

ResponseMessageWriter& ResponseMessageWriter::getInstance()
{
	static ResponseMessageWriter instance;
	return instance;
}

void ResponseMessageWriter::insertNewClient(int client_fd)
{
	this->writeBuffer[client_fd] = std::vector<unsigned char>();
}

void ResponseMessageWriter::deleteClient(int client_fd)
{
	this->writeBuffer.erase(client_fd);
}

