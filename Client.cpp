#include "Client.hpp"
#include "Server.hpp"
#include <iostream>
#include <sstream>
#include <unistd.h>

// constructors
Client::Client() :parseState(METHOD) {}
// destructor
Client::~Client() {}
// copy constructors
Client::Client(Client const& other)
    : client_fd(other.client_fd), server(other.server), req(other.req), res(other.res),
      readBuffer(other.readBuffer), sendBuffer(other.sendBuffer), parseState(METHOD) {}
// operators
Client& Client::operator=(Client const& rhs)
{
    if (this != &rhs)
    {
        this->res = rhs.res;
        this->req = rhs.req;
        this->client_fd = rhs.client_fd;
        this->server = rhs.server;
        this->readBuffer = rhs.readBuffer;
        this->sendBuffer = rhs.sendBuffer;
        this->parseState = rhs.parseState;
    }
    return *this;
}

// getter
ResponseMessage& Client::getRes(void)
{
    return this->res;
}

RequestMessage& Client::getReq(void)
{
    return this->req;
}

Server* Client::getServer(void) const
{
    return this->server;
}
// setter
void Client::setServer(Server* server)
{
    this->server = server;
}
void Client::setFd(int fd)
{
    this->client_fd = fd;
}
// functions
void Client::runServer()
{
    this->server->runServer(this->req, this->res);
}

void Client::errorEventProcess(void)
{
    std::cout << "errorEventProcess" << std::endl;
}

bool Client::readEventProcess(void)
{
    if (parseState == DONE || parseState == ERROR)
    {
        // 메시지 처리하여 버퍼에 입력해야함.
        // events.changeEvents(ident, EVFILT_WRITE, EV_ENABLE, 0, 0, NULL);
        std::cout << "메시지 잘 받았습니다^^" << std::endl;

        sendBuffer.insert(sendBuffer.end(),
                                                    "HTTP/1.1 404 Not Found\r\nServer: nginx/1.25.1\r\nDate: Fri, 28 Jul 2023 12:42:57 GMT\r\n\
                    Content-Type: text/html\r\nContent-Length: 153\r\nConnection: keep-alive\r\n\r\n<html>\r\n\
                    <head><title>404 Not Found</title></head>\r\n<body>\r\n<center><h1>Hello my name is jj!!</h1></center>\r\n\
                    <hr><center>webserv 0.1</center>\r\n</body>\r\n</html>",
                                                    &"HTTP/1.1 404 Not Found\r\nServer: nginx/1.25.1\r\nDate: Fri, 28 Jul 2023 12:42:57 GMT\r\n\
                    Content-Type: text/html\r\nContent-Length: 153\r\nConnection: keep-alive\r\n\r\n<html>\r\n\
                    <head><title>404 Not Found</title></head>\r\n<body>\r\n<center><h1>Hello my name is jj!!</h1></center>\r\n\
                    <hr><center>webserv 0.1</center>\r\n</body>\r\n</html>"[374]);
        parseState = METHOD;
        req.clear();
        return true;
    }
    return false;
}

bool Client::writeEventProcess(void)
{
    int writeSize = write(client_fd, &sendBuffer[0], sendBuffer.size());
    if (writeSize == -1)
    {
        std::cout << "write() error" << std::endl;
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

    switch (parseState)
    {
    case METHOD:
        readMethod(buffer);
        break;
    case REQUEST_TARGET:
        readRequestTarget(buffer);
        break;
    case HTTP_VERSION:
        readHttpVersion(buffer);
        break;
    case HEADER:
        readHeader(buffer);
        break;
    case BODY:
        readBody(buffer);
        break;
    default:
        break;
    }
    return false;
}

void Client::readHeader(const char* buffer)
{
    std::stringstream headerSstream;
    std::string line;
    std::string key;
    std::string value;
    std::vector<unsigned char>::iterator pos;

    readBuffer.insert(readBuffer.end(), buffer, buffer + strlen(buffer));

    while ((pos = std::search(readBuffer.begin(), readBuffer.end(), "\n", &"\n"[1])) != readBuffer.end())
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
                return;
            }
            else
            {
                parseState = BODY;
                return (readBody(""));
            }
        }
        headerSstream << line;
        getline(headerSstream, key, ':');
        if (key.size() == 0)
        {
            parseState = ERROR;
            return;
        }
        for (size_t i = 0; i < key.size(); i++)
        {
            if (isspace(key[i]))
            {
                parseState = ERROR;
                return;
            }
        }
        getline(headerSstream, value);
        if (value.size() == 0)
        {
            parseState = ERROR;
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


void Client::readBody(const char* buffer)
{
    std::vector<unsigned char>::iterator pos;

    if (req.method == "GET" || req.method == "DELETE")
    {
        parseState = DONE;
        return;
    }
    readBuffer.insert(readBuffer.end(), buffer, buffer + strlen(buffer));
    std::string a;
    if ((pos = std::search(readBuffer.begin(), readBuffer.end(), "\r\n\r\n", &"\r\n\r\n"[4])) != readBuffer.end())
    {
        req.body = std::vector<unsigned char>(readBuffer.begin(), pos);
        readBuffer.erase(readBuffer.begin(), pos + 4);
        parseState = DONE;
    }
}
void Client::readMethod(const char* buffer)
{
    std::vector<unsigned char>::iterator pos;

    // 첫줄에 개행만 들어온 경우 무시
    if (strcmp(buffer, "\n") == 0 || strcmp(buffer, "\r\n") == 0)
        return;
    // 입력버퍼벡터 뒤에 방금읽은 버퍼를 덧붙임
    readBuffer.insert(readBuffer.end(), buffer, buffer + strlen(buffer));
    // 입력버퍼벡터에서 공백을 찾음
    if ((pos = std::search(readBuffer.begin(), readBuffer.end(), " ", &" "[1])) != readBuffer.end())
    {
        req.method = std::string(readBuffer.begin(), pos);
        req.startLine = req.method;
        readBuffer.erase(readBuffer.begin(), pos + 1);
        parseState = REQUEST_TARGET;
        // 또 다른 공백을 찾은 경우 다음 파싱으로 넘어감.
        // 이때 공백이 연속해서 들어오는 경우를 생각해 볼 수 있는데 이런 경우 에러처리로 됨.
        if ((pos = std::search(readBuffer.begin(), readBuffer.end(), " ", &" "[1])) != readBuffer.end())
            readRequestTarget("");
        else if ((pos = std::search(readBuffer.begin(), readBuffer.end(), "\r\n", &"\r\n"[2])) != readBuffer.end())
        {
            parseState = ERROR;
        }
    }
    // 공백이 아닌 개행을 읽은 경우 파싱이 완료되지 못하기 때문에 에러
    else if ((pos = std::search(readBuffer.begin(), readBuffer.end(), "\r\n", &"\r\n"[2])) != readBuffer.end())
    {
        parseState = ERROR;
        readBuffer.erase(readBuffer.begin(), pos + 2);
    }
}

void Client::readRequestTarget(const char* buffer)
{
    std::vector<unsigned char>::iterator pos;

    readBuffer.insert(readBuffer.end(), buffer, buffer + strlen(buffer));
    if ((pos = std::search(readBuffer.begin(), readBuffer.end(), " ", &" "[1])) != readBuffer.end())
    {
        req.requestTarget = std::string(readBuffer.begin(), pos);
        req.startLine += " " + req.requestTarget;
        readBuffer.erase(readBuffer.begin(), pos + 1);
        parseState = HTTP_VERSION;
        if ((pos = std::search(readBuffer.begin(), readBuffer.end(), "\r\n", &"\r\n"[2])) != readBuffer.end())
            readHttpVersion("");
        else if ((pos = std::search(readBuffer.begin(), readBuffer.end(), " ", &" "[1])) != readBuffer.end())
        {
            parseState = ERROR;
        }
    }
    else if ((pos = std::search(readBuffer.begin(), readBuffer.end(), "\r\n", &"\r\n"[2])) != readBuffer.end())
    {
        parseState = ERROR;
        readBuffer.erase(readBuffer.begin(), pos + 2);
    }
}

void Client::readHttpVersion(const char* buffer)
{
    std::vector<unsigned char>::iterator pos;

    readBuffer.insert(readBuffer.end(), buffer, buffer + strlen(buffer));
    if ((pos = std::search(readBuffer.begin(), readBuffer.end(), "\r\n", &"\r\n"[2])) != readBuffer.end())
    {
        req.httpVersion = std::string(readBuffer.begin(), pos);
        readBuffer.erase(readBuffer.begin(), pos + 2);
        if (req.httpVersion != "HTTP/1.1" && req.httpVersion != "HTTP/1.0")
        {
            parseState = ERROR;
            return;
        }
        req.startLine += " " + req.httpVersion;
        parseState = HEADER;
        if ((pos = std::search(readBuffer.begin(), readBuffer.end(), "\r\n", &"\r\n"[2])) != readBuffer.end())
            readHeader("");
        return;
    }
}

bool Client::isSendBufferEmpty(void)
{
    return (sendBuffer.size() == 0);
}