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

// constructors
Client::Client() : parseState(READY) {
}
// destructor
Client::~Client() {}
// copy constructors
Client::Client(Client const& other)
	: client_fd(other.client_fd), server(other.server),
	  queRes(other.queRes), readBuffer(other.readBuffer),
	  sendBuffer(other.sendBuffer), parseState(METHOD) {}
// operators
Client& Client::operator=(Client const& rhs)
{
	if (this != &rhs)
	{
		this->queRes = rhs.queRes;
		this->client_fd = rhs.client_fd;
		this->server = rhs.server;
		this->readBuffer = rhs.readBuffer;
		this->sendBuffer = rhs.sendBuffer;
		this->parseState = rhs.parseState;
	}
	return *this;
}

// getter
ResponseMessage& Client::getBackRes(void) { return queRes.back(); }
ResponseMessage& Client::getFrontRes(void) { return queRes.front(); }


Server* Client::getServer(void) const { return this->server; }
// setter
void Client::setServer(Server* server) { this->server = server; }
void Client::setFd(int fd) { this->client_fd = fd; }

void Client::setEvents(Event* event) { this->events = event; };
// functions

ResponseMessage Client::popRes(void)
{
	ResponseMessage ret = queRes.front();
	queRes.pop();
	return ret;
}

void Client::errorEventProcess(void)
{
	std::cout << "errorEventProcess" << std::endl;
}

bool Client::readEventProcess(void) // RUN 5
{
	if (parseState == DONE || parseState == ERROR)
	{
		// 메시지 처리하여 버퍼에 입력해야함.
		// events.changeEvents(ident, EVFILT_WRITE, EV_ENABLE, 0, 0, NULL);
		std::cout << "메시지 잘 받았습니다^^" << std::endl;
		// START Parse Request Message //
		std::string toFindUri = httpRequestManager.getFrontReq().uri;
		std::cout << BOLDYELLOW << "파싱된 Sentence : " << toFindUri << RESET << '\n';
		std::string	foundUri;
		std::string foundFile;
		int NoUri = 0;
		
		std::cout << "--- URI LIST ---\n";
		std::map<std::string, Location> myLocations = this->getServer()->getLocations();
		std::map<std::string, Location>::iterator it;
		for (it = myLocations.begin(); it != myLocations.end(); ++it) 
		{
			std::cout << BOLDGREEN << "블록에 있는 URI 인자 : " << it->first << RESET << std::endl;
			std::cout << "It->first : " << it->first << ' ' << " TOFINDURI : " << toFindUri << '\n';
			if (it->first == "/" && toFindUri == "/")
			{
				foundUri = it->first;
				break ;
			}
			for (size_t i = 0; i < it->first.size(); i++)
			{
				NoUri = 0;
				if (it->first.size() == 1 && it->first == "/")
				{
					NoUri = 1;
					break ;
				}
				if (i < it->first.size() && it->first[i] != toFindUri[i])
					NoUri = 1;
			}
			if (NoUri == 0)
			{
				foundUri = it->first;
				std::cout << BOLDCYAN << "FOUND URI : " << foundUri << '\n';
				break ;
			}
			else
				std::cout << "NO URI matching with sentence\n";
		}
		std::cout << "No Uri : " << NoUri << '\n' << "No Uri 0이면 일치하는 블록 있는거고 1이면 없는 겁니다. 1일때 예외처리 해줘야 합니다.\n";
		std::cout << "----------------\n";
		
		if (NoUri == 0)
		{
			int size = it->first.size();
			foundFile = toFindUri.substr(size, 10240);
			if (foundFile.empty())
				std::cout << BOLDRED << "foundfile is empty...\n" << RESET;
			else
				std::cout << BOLDCYAN << "FOUND FILE : " << foundFile << RESET << '\n';
		}

		int block = 0;
		std::vector<std::string> list;
		if (foundFile.empty())
		{
			list = this->getServer()->getLocations()[foundUri].getIndex();
		}
		else
		{
			block = 1;
			list.push_back(foundFile);
		}		
		std::string rootie = this->getServer()->getLocations()[foundUri].getRoot();
		bool isAutoIndex = this->getServer()->getLocations()[foundUri].getAutoIndex();
		list.push_back(rootie);
		list.push_back(std::to_string(isAutoIndex));

		httpRequestManager.setHandler(list);
		httpRequestManager.dynamicOpenFd(*this);
		httpRequestManager.sendReqtoEvent(*this);
		httpRequestManager.dynamicRunCgi(*this);
		std::cout << BOLDCYAN << " -- SUCCESSFULLY SEND MESSAGE -- \n\n";
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
	int writeSize = write(client_fd, &sendBuffer[0], sendBuffer.size());
	if (writeSize == -1)
	{
		std::cout << "write() error" << std::endl;
		std::cout << errno << std::endl;
		return true;
	}
	sendBuffer.erase(sendBuffer.begin(), sendBuffer.begin() + writeSize);
	return false;
}

bool Client::readMessage(void)
{
	const size_t BUFFER_SIZE = 1024;
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
		createRequest();
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
	HttpRequest& req = httpRequestManager.getBackReq();

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
			if (req.headers.find("host") == req.headers.end())
			{
				parseState = ERROR;
				req.errorCode = BAD_REQUEST;
				return;
			}
			else if (req.headers.find("content-length") == req.headers.end() &&
					 (req.method == "GET" || req.method == "DELETE"))
			{
				parseState = DONE;
				return;
			}
			else
			{
				// header Parsing이 끝난 후, flag를 done이나 BODY 가 아닌 CHUNKED 로
				// 보내기 위한 로직
				std::map<std::string, std::string>::iterator encodingIt =
					req.headers.find("transfer-encoding");
				if (encodingIt != req.headers.end() &&
					encodingIt->second == "chunked")
				{
					// parse State 변경
					parseState = CHUNKED;
					return (readChunked("", 0));
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
			req.errorCode = BAD_REQUEST;
			return;
		}
		for (size_t i = 0; i < key.size(); i++)
		{
			if (isspace(key[i]))
			{
				parseState = ERROR;
				req.errorCode = BAD_REQUEST;
				return;
			}
		}
		getline(headerSstream, value);
		if (value.size() == 0)
		{
			parseState = ERROR;
			req.errorCode = BAD_REQUEST;
			return;
		}
		headerSstream.clear();
		// value의 처음 ifs들을 지움 근데 하나가 아닐 수 있음
		while (value[0] == ' ')
			value.erase(value.begin());
		for (size_t i = 0; i < key.size(); i++)

			key[i] = tolower(key[i]);
		req.headers[key] = value;
	}
}

void Client::readChunked(const char* buffer, size_t readSize)
{
	static const char* crlf = "\r\n";
	static std::string strbodySize;
	static long longBodySize;
	static bool haveToReadBody = false;
	HttpRequest& req = httpRequestManager.getBackReq();

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
			req.body.insert(req.body.end(), readBuffer.begin(),
							readBuffer.begin() + longBodySize);
			haveToReadBody = false;
			if (readBuffer[longBodySize] != '\r' ||
				readBuffer[longBodySize + 1] != '\n')
			{
				parseState = ERROR;
				req.errorCode = BAD_REQUEST;
				return;
			}
			readBuffer.erase(readBuffer.begin(),
							 readBuffer.begin() + longBodySize + 2);
			pos = std::search(readBuffer.begin(), readBuffer.end(), crlf, &crlf[2]);
		}
	}
}

void Client::readBody(const char* buffer, size_t readSize)
{
	std::vector<unsigned char>::iterator pos;
	HttpRequest& req = httpRequestManager.getBackReq();

	readBuffer.insert(readBuffer.end(), buffer, buffer + readSize);
	if (req.headers.find("content-length") != req.headers.end())
	{
		size_t lengthToRead =
			atoi(req.headers["content-length"].c_str()) - req.body.size();
		if (lengthToRead > readBuffer.size())
		{
			req.body.insert(req.body.end(), readBuffer.begin(), readBuffer.end());
			readBuffer.clear();
		}
		else
		{
			req.body.insert(req.body.end(), readBuffer.begin(),
							readBuffer.begin() + lengthToRead);
			readBuffer.erase(readBuffer.begin(), readBuffer.begin() + lengthToRead);
			parseState = DONE;
		}
	}
	else if ((pos = std::search(readBuffer.begin(), readBuffer.end(),
								"\r\n\r\n", &"\r\n\r\n"[4])) !=
			 readBuffer.end())
	{
		req.body.insert(req.body.end(), readBuffer.begin(), pos);
		readBuffer.erase(readBuffer.begin(), pos + 4);
		parseState = DONE;
	}
}
void Client::readMethod(const char* buffer)
{
	std::vector<unsigned char>::iterator pos;
	HttpRequest& req = httpRequestManager.getBackReq();

	// 첫줄에 개행만 들어온 경우 무시
	if (strcmp(buffer, "\n") == 0 || strcmp(buffer, "\r\n") == 0)
		return;
	// 입력버퍼벡터 뒤에 방금읽은 버퍼를 덧붙임
	readBuffer.insert(readBuffer.end(), buffer, buffer + strlen(buffer));
	// 입력버퍼벡터에서 공백을 찾음
	if ((pos = std::search(readBuffer.begin(), readBuffer.end(), " ", &" "[1])) !=
		readBuffer.end())
	{
		req.method = std::string(readBuffer.begin(), pos);
		req.startLine = req.method;
		readBuffer.erase(readBuffer.begin(), pos + 1);
		parseState = URI;
		// 또 다른 공백을 찾은 경우 다음 파싱으로 넘어감.
		// 이때 공백이 연속해서 들어오는 경우를 생각해 볼 수 있는데 이런 경우
		// 에러처리로 됨.
		if ((pos = std::search(readBuffer.begin(), readBuffer.end(), " ",
							   &" "[1])) != readBuffer.end())
			readUri("");
		else if ((pos = std::search(readBuffer.begin(), readBuffer.end(), "\r\n",
									&"\r\n"[2])) != readBuffer.end())
		{
			parseState = ERROR;
			req.errorCode = BAD_REQUEST;
		}
	}
	// 공백이 아닌 개행을 읽은 경우 파싱이 완료되지 못하기 때문에 에러
	else if ((pos = std::search(readBuffer.begin(), readBuffer.end(), "\r\n",
								&"\r\n"[2])) != readBuffer.end())
	{
		parseState = ERROR;
		req.errorCode = BAD_REQUEST;
		readBuffer.erase(readBuffer.begin(), pos + 2);
	}
}

void Client::readUri(const char* buffer)
{
	std::vector<unsigned char>::iterator pos;
	HttpRequest& req = httpRequestManager.getBackReq();

	readBuffer.insert(readBuffer.end(), buffer, buffer + strlen(buffer));
	if ((pos = std::search(readBuffer.begin(), readBuffer.end(), " ", &" "[1])) !=
		readBuffer.end())
	{
		req.uri = std::string(readBuffer.begin(), pos);
		req.startLine += " " + req.uri;
		readBuffer.erase(readBuffer.begin(), pos + 1);
		parseState = HTTP_VERSION;
		if ((pos = std::search(readBuffer.begin(), readBuffer.end(), "\r\n",
							   &"\r\n"[2])) != readBuffer.end())
			readHttpVersion("");
		else if ((pos = std::search(readBuffer.begin(), readBuffer.end(), " ",
									&" "[1])) != readBuffer.end())
		{
			parseState = ERROR;
			req.errorCode = BAD_REQUEST;
		}
	}
	else if ((pos = std::search(readBuffer.begin(), readBuffer.end(), "\r\n",
								&"\r\n"[2])) != readBuffer.end())
	{
		parseState = ERROR;
		req.errorCode = BAD_REQUEST;
		readBuffer.erase(readBuffer.begin(), pos + 2);
	}
}

void Client::readHttpVersion(const char* buffer)
{
	std::vector<unsigned char>::iterator pos;
	HttpRequest& req = httpRequestManager.getBackReq();

	readBuffer.insert(readBuffer.end(), buffer, buffer + strlen(buffer));
	if ((pos = std::search(readBuffer.begin(), readBuffer.end(), "\r\n",
						   &"\r\n"[2])) != readBuffer.end())
	{
		req.httpVersion = std::string(readBuffer.begin(), pos);
		readBuffer.erase(readBuffer.begin(), pos + 2);
		if (req.httpVersion != "HTTP/1.1" && req.httpVersion != "HTTP/1.0")
		{
			parseState = ERROR;
			req.errorCode = HTTP_VERSION_NOT_SUPPORT;
			return;
		}
		req.startLine += " " + req.httpVersion;
		parseState = HEADER;
		if ((pos = std::search(readBuffer.begin(), readBuffer.end(), "\r\n",
							   &"\r\n"[2])) != readBuffer.end())
			readHeader("");
		return;
	}
}

bool Client::isSendBufferEmpty(void)
{
	return (sendBuffer.size() == 0);
}

void Client::createRequest(void)
{
	httpRequestManager.pushReq();
}

void Client::createResponse(void)
{
	queRes.push(ResponseMessage());
}
