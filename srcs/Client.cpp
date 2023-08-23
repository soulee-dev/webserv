#include "Client.hpp"
#include "Http/HttpRequestManager.hpp"
#include "Server.hpp"
#include <algorithm>
#include <fcntl.h>
#include "Color.hpp"
#include "Http/HttpRequestManager.hpp"
#include "Http/Handler/ErrorHandler.hpp"
#include <iostream>
#include <sstream>
#include <unistd.h>
#include "ConfigParser.hpp"

// constructors
Client::Client() : parseState(READY), haveToReadBody(false), writeIndex(0) {
	readBuffer.reserve(100000000);
	sendBuffer.reserve(100000000);
}
// destructor
Client::~Client() {}
// copy constructors
Client::Client(Client const& other)
	: client_fd(other.client_fd), server(other.server),
	  request(other.request), response(other.response), readBuffer(other.readBuffer),
	  sendBuffer(other.sendBuffer), parseState(METHOD), writeIndex(other.writeIndex) {}
// operators
Client& Client::operator=(Client const& rhs)
{
	if (this != &rhs)
	{
		this->client_fd = rhs.client_fd;
		this->server = rhs.server;
		this->readBuffer = rhs.readBuffer;
		this->sendBuffer = rhs.sendBuffer;
		this->parseState = rhs.parseState;
		this->writeIndex = rhs.writeIndex;
		this->request = rhs.request;
		this->response = rhs.response;
	}
	return *this;
}

// getter
Server* Client::getServer(void) const { return this->server; }
// setter
void Client::setServer(Server* server) { this->server = server; }
void Client::setFd(int fd) { this->client_fd = fd; }

void Client::setEvents(Event* event) { this->events = event; };
// functions

void Client::errorEventProcess(void)
{
	std::cout << "errorEventProcess" << std::endl;
}

bool Client::readEventProcess(void) // RUN 5
{
	if (parseState == DONE)
	{
		// 메시지 처리하여 버퍼에 입력해야함.
		// events.changeEvents(ident, EVFILT_WRITE, EV_ENABLE, 0, 0, NULL);
		std::cout << BOLDGREEN << "URI : " << request.uri << RESET << '\n';
		std::cout << BOLDYELLOW << "HTTP VERSION : " << request.httpVersion << RESET << '\n';

		httpRequestManager.SetHandler(*this);
		httpRequestManager.DynamicOpenFd(*this);
		httpRequestManager.SendReqtoEvent(*this);
		httpRequestManager.DynamicRunCgi(*this);
		std::cout << BOLDCYAN << " -- SUCCESSFULLY GET MESSAGE -- \n\n" << RESET;
		parseState = READY;
		return true;
	}
	else if (parseState == ERROR)
	{
		// request.errorCode = METHOD_NOT_ALLOWED;
		std::cout << "ERROR : " << request.errorCode << '\n';
		ErrorHandler::sendReqtoError(*this);
		std::cout << BOLDCYAN << " -- SUCCESSFULLY GET MESSAGE -- \n\n" << RESET;
		parseState = READY;
		return true;
	}
	return false;
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
		sendBuffer.clear();
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

	// 함수 포인터 배열 사용해보는것도?
	switch (parseState)
	{
	case READY:
		request.clear();
	case METHOD:
		readMethod(buffer);
		break;
	case URI:
		readUri(buffer);
		break;
	case HTTP_VERSION:
		readHttpVersion(buffer);
		break;
	case HEADER:
		readHeader(buffer);
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
// client 안에 readMsg 와 writeMsg handle 할 각각의 클래스를 넣어두고 써보는
// 것도 ..?
void Client::readHeader(const char* buffer)
{
	std::stringstream headerSstream;
	std::string line;
	std::string key;
	std::string value;
	std::vector<unsigned char>::iterator pos;

	readBuffer.insert(readBuffer.end(), buffer, buffer + strlen(buffer));

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
				std::cout<<"DEBUG1\n";
				request.errorCode = BAD_REQUEST;
				return;
			}
			else if (checkMethod(request.method))
			{
				parseState = ERROR;
				std::cout << "DEBUG CHECKMETHOD\n";
				request.errorCode = METHOD_NOT_ALLOWED; // 이 경우, 405번을 부여하지 않으면 테스트에서 통과가 불가능합니다(원래 400).
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
				// header Parsing이 끝난 후, flag를 done이나 BODY 가 아닌 CHUNKED 로
				// 보내기 위한 로직
				std::map<std::string, std::string>::iterator encodingIt =
					request.headers.find("transfer-encoding");
				if (encodingIt != request.headers.end() &&
					encodingIt->second == "chunked")
				{
					// parse State 변경
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
			std::cout<<"DEBUG3\n";
			request.errorCode = BAD_REQUEST;
			return;
		}
		for (size_t i = 0; i < key.size(); i++)
		{
			if (isspace(key[i]))
			{
				parseState = ERROR;
				std::cout<<"DEBUG4\n";
				request.errorCode = BAD_REQUEST;
				return;
			}
		}
		getline(headerSstream, value);
		if (value.size() == 0)
		{
			parseState = ERROR;
			std::cout<<"DEBUG5\n";
			request.errorCode = BAD_REQUEST;
			return;
		}
		headerSstream.clear();
		// value의 처음 ifs들을 지움 근데 하나가 아닐 수 있음
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
				std::cout<<"DEBUG6\n";
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

bool Client::checkMethod(std::string const& method)
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

void Client::readMethod(const char* buffer)
{
	std::vector<unsigned char>::iterator pos;

	// 입력버퍼벡터 뒤에 방금읽은 버퍼를 덧붙임
	readBuffer.insert(readBuffer.end(), buffer, buffer + strlen(buffer));
	while (readBuffer.size() > 1 && readBuffer[0] == '\r' && readBuffer[1] == '\n')
		readBuffer.erase(readBuffer.begin(), readBuffer.begin() + 2);
	if (readBuffer.size() == 0)
		return ;
	// 입력버퍼벡터에서 공백을 찾음
	if ((pos = std::search(readBuffer.begin(), readBuffer.end(), " ", &" "[1])) !=
		readBuffer.end())
	{
		request.method = std::string(readBuffer.begin(), pos);
		request.startLine = request.method;
		readBuffer.erase(readBuffer.begin(), pos + 1);
		parseState = URI;
		// 또 다른 공백을 찾은 경우 다음 파싱으로 넘어감.
		// 이때 공백이 연속해서 들어오는 경우를 생각해 볼 수 있는데 이런 경우
		// 에러처리로 됨.
		if ((pos = std::search(readBuffer.begin(), readBuffer.end(), " ",
							   &" "[1])) != readBuffer.end())
			readUri("");
		else if ((pos = std::search(readBuffer.begin(), readBuffer.end(), CRLF,
									&CRLF[2])) != readBuffer.end())
		{
			parseState = ERROR;
			std::cout<<"DEBUG7\n";
			request.errorCode = BAD_REQUEST;
		}
	}
	// 공백이 아닌 개행을 읽은 경우 파싱이 완료되지 못하기 때문에 에러
	else if ((pos = std::search(readBuffer.begin(), readBuffer.end(), CRLF,
								&CRLF[2])) != readBuffer.end())
	{
		std::cout << "METHOD : " << request.method << std::endl;
		std::cout << "URI : " << request.uri << std::endl;
		std::cout << "PROTO : " << request.httpVersion << std::endl;
		parseState = ERROR;
		std::cout<<"DEBUG8\n";
		request.errorCode = METHOD_NOT_ALLOWED; // 이 경우 또한 405번을 부여하지 않으면 테스트에서 통과가 불가능합니다(원래 400).
		readBuffer.erase(readBuffer.begin(), pos + 2);
	}
}

void Client::readUri(const char* buffer)
{
	std::vector<unsigned char>::iterator pos;

	readBuffer.insert(readBuffer.end(), buffer, buffer + strlen(buffer));
	if ((pos = std::search(readBuffer.begin(), readBuffer.end(), " ", &" "[1])) !=
		readBuffer.end())
	{
		request.uri = std::string(readBuffer.begin(), pos);
		request.startLine += " " + request.uri;
		readBuffer.erase(readBuffer.begin(), pos + 1);
		parseState = HTTP_VERSION;
		if ((pos = std::search(readBuffer.begin(), readBuffer.end(), CRLF,
							   &CRLF[2])) != readBuffer.end())
			readHttpVersion("");
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
		std::cout<<"DEBUG9\n";
		request.errorCode = BAD_REQUEST;
		readBuffer.erase(readBuffer.begin(), pos + 2);
	}
}

void Client::readHttpVersion(const char* buffer)
{
	std::vector<unsigned char>::iterator pos;

	readBuffer.insert(readBuffer.end(), buffer, buffer + strlen(buffer));
	if ((pos = std::search(readBuffer.begin(), readBuffer.end(), CRLF,
						   &CRLF[2])) != readBuffer.end())
	{
		request.httpVersion = std::string(readBuffer.begin(), pos);
		readBuffer.erase(readBuffer.begin(), pos + 2);
		if (request.httpVersion != "HTTP/1.1" && request.httpVersion != "HTTP/1.0")
		{
			parseState = ERROR;
			std::cout<<"DEBUG10\n";
			request.errorCode = HTTP_VERSION_NOT_SUPPORT;
			return;
		}
		request.startLine += " " + request.httpVersion;
		parseState = HEADER;
		if ((pos = std::search(readBuffer.begin(), readBuffer.end(), CRLF,
							   &CRLF[2])) != readBuffer.end())
			readHeader("");
		return;
	}
}

bool Client::isSendBufferEmpty(void)
{
	return (sendBuffer.size() == 0);
}
