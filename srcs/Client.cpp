#include "Client.hpp"
#include "Server.hpp"
#include "Color.hpp"
#include "Http/HttpRequestManager.hpp"
#include "Http/Handler/ErrorHandler.hpp"
#include "Config/ConfigParser.hpp"
#include <algorithm>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <sys/event.h>
#include <unistd.h>

Client::Client() : parseState(READY), haveToReadBody(false), writeIndex(0) {
	readBuffer.reserve(100000000);
	sendBuffer.reserve(100000000);
	request.clear();
	response.clear();
}

Client::~Client() {}

Server* Client::getServer(void) const { return this->server; }

void Client::setServer(Server* server) { this->server = server; }
void Client::setFd(int fd) { this->client_fd = fd; }
void Client::setEvents(Event* event) { this->events = event; };

void Client::errorEventProcess(void)
{
	std::cout << "errorEventProcess" << std::endl;
}

bool Client::readEventProcess(void)
{
	if (parseState == DONE)
	{
		httpRequestManager.Handle(*this);
		std::cout << BOLDCYAN << " -- SUCCESSFULLY GET MESSAGE -- \n\n" << RESET;
		parseState = READY;
	}
	else if (parseState == ERROR)
	{
		std::cout << "ERROR : " << request.errorCode << '\n';
		HandleError(*this, request.errorCode);
		std::cout << BOLDCYAN << " -- SUCCESSFULLY GET MESSAGE -- \n\n" << RESET;
		parseState = READY;
	}
	else
		return false;
	// Dynamic인 경우 Handle안에서 EVFILT를 걸어주기 때문에 해줄필요 없다.
	if (request.is_static || parseState == ERROR)
	{
		if (request.file_fd != -1 && !request.is_put)
			events->changeEvents(request.file_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, this);
		else if (request.file_fd != -1 && request.is_put)
		{
			events->changeEvents(request.file_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, this);
		}
		else
			events->changeEvents(getClientFd(), EVFILT_WRITE, EV_ENABLE, 0, 0, this);
	}
	return true;
}

int Client::getClientFd(void) const
{
	return client_fd;
}

bool Client::writeEventProcess(void)
{
	const int size = sendBuffer.size() - writeIndex;
	int writeSize = write(client_fd, &sendBuffer[writeIndex], size);
	if (writeSize == -1)
	{
		std::cout << "write() error" << std::endl;
		std::cout << errno << std::endl;
		return true;
	}
	writeIndex += writeSize;
	if (writeIndex == sendBuffer.size())
	{
		writeIndex = 0;
		sendBuffer.clear();;
	}
	return false;
}

bool Client::readMessage(void)
{
	const size_t BUFFER_SIZE = 65536;
	char buffer[BUFFER_SIZE + 1];
	ssize_t readSize = read(client_fd, buffer, BUFFER_SIZE);

	if (readSize <= 0)
	{
		if (readSize == -1)
			std::cout << "read() error" << std::endl;
		return true;
	}
	buffer[readSize] = '\0';

	switch (parseState)
	{
	case READY:
		request.clear();
		response.clear();;
	case METHOD:
		readMethod(buffer, readSize);
		break;
	case URI:
		readUri(buffer, readSize);
		break;
	case HTTP_VERSION:
		readHttpVersion(buffer, readSize);
		break;
	case HEADER:
		readHeader(buffer, readSize);
		break;
	case BODY:
		readBody(buffer, readSize);
		break;
	case CHUNKED:
		readChunked(buffer, readSize);
	default:
		break;
	}
	return false;
}

void Client::readHeader(const char* buffer, size_t readSize)
{
	std::stringstream headerSstream;
	std::string line;
	std::string key;
	std::string value;
	std::vector<unsigned char>::iterator pos;

	readBuffer.insert(readBuffer.end(), buffer, buffer + readSize);
	while ((pos = std::search(readBuffer.begin(), readBuffer.end(), "\n",
							  &"\n"[1])) != readBuffer.end())
	{
		line = std::string(readBuffer.begin(), pos);
		readBuffer.erase(readBuffer.begin(), pos + 1);
		if (line[line.size() - 1] == '\r')
			line.pop_back();
		if (line.empty())
		{
			if (request.headers.find("host") == request.headers.end())
			{
				parseState = ERROR;
				request.errorCode = BAD_REQUEST;
				return;
			}
			else if (checkMethod())
			{
				parseState = ERROR;
				request.errorCode = METHOD_NOT_ALLOWED;
				return;
			}
			else if (request.headers.find("content-length") == request.headers.end() &&
					 (request.method == "GET" || request.method == "DELETE" || request.method == "HEAD"))
			{
				parseState = DONE;
				return;
			}
			else
			{
				std::map<std::string, std::string>::iterator encodingIt =
					request.headers.find("transfer-encoding");
				if (encodingIt != request.headers.end() &&
					encodingIt->second == "chunked")
				{
					parseState = CHUNKED;
					return (readChunked("", 0));
				}
				else if (request.method == "POST" && request.headers.find("content-length") == request.headers.end())
				{
					parseState = DONE;
					return ;
				}
				else
				{
					parseState = BODY;
					return (readBody("", 0));
				}
			}
		}
		headerSstream << line;
		getline(headerSstream, key, ':');
		if (key.size() == 0)
		{
			parseState = ERROR;
			request.errorCode = BAD_REQUEST;
			return;
		}
		for (size_t i = 0; i < key.size(); i++)
		{
			if (isspace(key[i]))
			{
				parseState = ERROR;
				request.errorCode = BAD_REQUEST;
				return;
			}
		}
		getline(headerSstream, value);
		if (value.size() == 0)
		{
			parseState = ERROR;
			request.errorCode = BAD_REQUEST;
			return;
		}
		headerSstream.clear();
		while (value[0] == ' ')
			value.erase(value.begin());
		for (size_t i = 0; i < key.size(); i++)
			key[i] = tolower(key[i]);
		request.headers[key] = value;
	}
}

void Client::readChunked(const char* buffer, size_t readSize)
{
	static const char* crlf = CRLF;

	readBuffer.insert(readBuffer.end(), buffer, buffer + readSize);
	std::vector<unsigned char>::iterator pos =
		std::search(readBuffer.begin(), readBuffer.end(), crlf, &crlf[2]);
	while (pos != readBuffer.end())
	{
		if (haveToReadBody == false)
		{
			strbodySize = std::string(readBuffer.begin(), pos);
			readBuffer.erase(readBuffer.begin(), pos + 2);
			longBodySize = strtol(strbodySize.c_str(), NULL, 16);
			haveToReadBody = true;
		}
		if (haveToReadBody == true)
		{
			if (longBodySize == 0)
			{
				parseState = DONE;
				haveToReadBody = false;
				return;
			}
			else if (longBodySize + 2 > readBuffer.size())
				return;
			request.body.insert(request.body.end(), readBuffer.begin(),
							readBuffer.begin() + longBodySize);
			haveToReadBody = false;
			if (readBuffer[longBodySize] != '\r' ||
				readBuffer[longBodySize + 1] != '\n')
			{
				parseState = ERROR;
				request.errorCode = BAD_REQUEST;
				return;
			}
			readBuffer.erase(readBuffer.begin(),
							 readBuffer.begin() + longBodySize + 2);
			pos = std::search(readBuffer.begin(), readBuffer.end(), crlf, &crlf[2]);
		}
	}
}

size_t minLen(size_t a, size_t b)
{
  if (a > b)
    return b;
  else
    return a;
}

void Client::readBody(const char* buffer, size_t readSize)
{
	std::vector<unsigned char>::iterator pos;

	readBuffer.insert(readBuffer.end(), buffer, buffer + readSize);
	if (request.headers.find("content-length") != request.headers.end())
	{
        size_t contentLen = atoi(request.headers["content-length"].c_str());
        size_t maxLen = minLen(contentLen, server->getClientBodySize());
        size_t lengthToRead = maxLen - request.body.size();
		if (lengthToRead > readBuffer.size())
		{
			request.body.insert(request.body.end(), readBuffer.begin(), readBuffer.end());
			readBuffer.clear();
		}
		else
		{
			request.body.insert(request.body.end(), readBuffer.begin(),
							readBuffer.begin() + lengthToRead);
			readBuffer.erase(readBuffer.begin(), readBuffer.begin() + lengthToRead);
			parseState = DONE;
		}
	}
	else if ((pos = std::search(readBuffer.begin(), readBuffer.end(),
								"\r\n\r\n", &"\r\n\r\n"[4])) !=
			 readBuffer.end())
	{
		request.body.insert(request.body.end(), readBuffer.begin(), pos);
		readBuffer.erase(readBuffer.begin(), pos + 4);
		parseState = DONE;
	}
}

bool Client::checkMethod(void)
{
	std::string tmp_uri;
	std::string		found_uri;
	bool is_found = false;
	size_t			location_pos;
	std::map<std::string, Location> locations = getServer()->getLocations();
	std::map<std::string, Location>::iterator location;
	size_t allow_methods;

	if (request.uri[request.uri.size() - 1] != '/')
		request.uri += "/";
	tmp_uri = request.uri;
	while (tmp_uri != "/")
	{
		if (is_found)
			break ;
		location_pos = tmp_uri.find_last_of('/');
		if (location_pos == std::string::npos)
			break;
		if (location_pos == 0)
			tmp_uri = "/";
		else
			tmp_uri = std::string(tmp_uri.begin(), tmp_uri.begin() + location_pos);
		for (location = locations.begin(); location != locations.end(); ++location)
		{
			if (tmp_uri == location->first)
			{
				found_uri = location->first;
				is_found = true;
				break;
			}
		}
	}
	if (!is_found)
		found_uri = "/";

	allow_methods = getServer()->getLocations()[found_uri].getAllowMethod();
	size_t my_method;
	if (request.method == "GET")
		my_method = GET;
	else if (request.method == "POST")
		my_method = POST;
	else if (request.method == "PUT")
		my_method = PUT;
	else if (request.method == "DELETE")
		my_method = DELETE;
	else if (request.method == "HEAD")
		my_method = HEAD;
	else
		return true;

	if ((allow_methods & my_method) != 0)
		return false;
	else
		return true;
}

void Client::readMethod(const char* buffer, size_t readSize)
{
	std::vector<unsigned char>::iterator pos;

	readBuffer.insert(readBuffer.end(), buffer, buffer + readSize);
	while (readBuffer.size() > 1 && readBuffer[0] == '\r' && readBuffer[1] == '\n')
		readBuffer.erase(readBuffer.begin(), readBuffer.begin() + 2);
	if (readBuffer.size() == 0)
		return ;
	if ((pos = std::search(readBuffer.begin(), readBuffer.end(), " ", &" "[1])) !=
		readBuffer.end())
	{
		request.method = std::string(readBuffer.begin(), pos);
		readBuffer.erase(readBuffer.begin(), pos + 1);
		parseState = URI;
		if ((pos = std::search(readBuffer.begin(), readBuffer.end(), " ",
							   &" "[1])) != readBuffer.end())
			readUri("", 0);
		else if ((pos = std::search(readBuffer.begin(), readBuffer.end(), CRLF,
									&CRLF[2])) != readBuffer.end())
		{
			parseState = ERROR;
			request.errorCode = BAD_REQUEST;
		}
	}
	else if ((pos = std::search(readBuffer.begin(), readBuffer.end(), CRLF,
								&CRLF[2])) != readBuffer.end())
	{
		parseState = ERROR;
		request.errorCode = METHOD_NOT_ALLOWED;
		readBuffer.erase(readBuffer.begin(), pos + 2);
	}
}

void Client::readUri(const char* buffer, size_t readSize)
{
	std::vector<unsigned char>::iterator pos;

	readBuffer.insert(readBuffer.end(), buffer, buffer + readSize);
	if ((pos = std::search(readBuffer.begin(), readBuffer.end(), " ", &" "[1])) !=
		readBuffer.end())
	{
		request.uri = std::string(readBuffer.begin(), pos);
		readBuffer.erase(readBuffer.begin(), pos + 1);
		parseState = HTTP_VERSION;
		if ((pos = std::search(readBuffer.begin(), readBuffer.end(), CRLF,
							   &CRLF[2])) != readBuffer.end())
			readHttpVersion("", 0);
		else if ((pos = std::search(readBuffer.begin(), readBuffer.end(), " ",
									&" "[1])) != readBuffer.end())
		{
			request.errorCode = BAD_REQUEST;
		}
	}
	else if ((pos = std::search(readBuffer.begin(), readBuffer.end(), CRLF,
								&CRLF[2])) != readBuffer.end())
	{
		parseState = ERROR;
		request.errorCode = BAD_REQUEST;
		readBuffer.erase(readBuffer.begin(), pos + 2);
	}
}

void Client::readHttpVersion(const char* buffer, size_t readSize)
{
	std::vector<unsigned char>::iterator pos;

	readBuffer.insert(readBuffer.end(), buffer, buffer + readSize);
	if ((pos = std::search(readBuffer.begin(), readBuffer.end(), CRLF,
						   &CRLF[2])) != readBuffer.end())
	{
		request.http_version = std::string(readBuffer.begin(), pos);
		readBuffer.erase(readBuffer.begin(), pos + 2);
		if (request.http_version != "HTTP/1.1" && request.http_version != "HTTP/1.0")
		{
			parseState = ERROR;
			request.errorCode = HTTP_VERSION_NOT_SUPPORT;
			return;
		}
		parseState = HEADER;
		if ((pos = std::search(readBuffer.begin(), readBuffer.end(), CRLF,
							   &CRLF[2])) != readBuffer.end())
			readHeader("", 0);
		return;
	}
}

bool Client::isSendBufferEmpty(void)
{
	return (sendBuffer.size() == 0);
}
