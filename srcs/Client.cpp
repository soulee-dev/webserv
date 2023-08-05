#include "Client.hpp"
#include "Server.hpp"
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <unistd.h>

// constructors
Client::Client() : parseState(METHOD) {}
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

void Client::setEvents(Event* event)
{
    this->events = event;
};
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
        // events->changeEvents(2, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, &sendBuffer);
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
        std::cout << "!!write() error" << std::endl;
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
    case CHUNKED:
        readChunked(buffer);
    default:
        break;
    }
    return false;
}
// client 안에 readMsg 와 writeMsg handle 할 각각의 클래스를 넣어두고 써보는 것도 ..?
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

                // header Parsing이 끝난 후, flag를 done이나 BODY 가 아닌 CHUNKED 로 보내기 위한 로직
                if (req.headers.find("transfer-encoding") != req.headers.end())
                {
                    // make_tempfile_name;
                    std::string tempFile = "temp_" + req.requestTarget;
                    // file open
                    req.chunkedFd = open(tempFile.c_str(), O_WRONLY | O_APPEND, 0644);
                    // fd event 등록 ?
                    events->changeEvents(req.chunkedFd, EVFILT_WRITE, EV_ADD | EV_DISABLE, 0, 0, NULL);

                    // parse State 변경
                    parseState = CHUNKED;
                    return (readChunked(""));
                }
                else
                {
                    parseState = BODY;
                    return (readBody(""));
                }
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

void Client::readChunked(const char* buffer)
{
    if (req.chunkedFd == -1)
        std::cout << "wrong parseState" << std::endl;
    readBuffer.insert(readBuffer.end(), buffer, buffer + strlen(buffer));

    std::vector<unsigned char>::iterator pos;
    // 반복문을 돌면서 readBuffer를 읽음 -> 처음엔 무조건 size, 다음줄은 무조건 data여야 함
    // 반복문의 종료 조건 :  마지막 \r\n까지
    // 반복 조건 : 1\r\n 2\r\n이 한 쌍
    // 반복 시행 :
    // 1번째 줄은 16진수 숫자
    // 숫자 변환 법 (택1):
    // 1. istringstream
    // std::istringstream iss(s);
    // iss >> std::hex >> i;
    // 2. sscanf이용
    // 3. 삼항 연산자 사용 https://commandlinefanatic.com/cgi-bin/showarticle.cgi?article=art077
    // 완전 종료 조건 : 사이즈가 0
    // 이경우 FLAG를 DONE 으로 바꿈,
    // write event 의 filter도변경
    // 2번째 줄
    // data는 위에서 읽은 사이즈 만큼만 읽기
    // 만약 읽은 사이즈보다 작은 데이터만 남아있으면 에러
    // 왜냐하면 마지막 데이터도 \r\n이 붙어 있는 상태의 것이기 때문

    // readBuffer 에 1번째줄, 2번째쭐 1쌍 이상이 있지 않으면 readBuffer에 붙여 넣기만 하고 끝남
    //  만약 계속 아무것도 안들어 온다면 타임 아웃.

    // 마지막 \r\n 이후에 남은 데이터가 있다면 readBuffer에 붙임;

    // write 이벤트 등록, fd 로 write 는 nonClientWriteEventProcess 에서 함,
    // cgi 에 보내는 것도 해당 프로세스가 처리하도록 할 것임
    // udata에 있는 readBuffer를 write 하도록 함
    events->changeEvents(req.chunkedFd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, &readBuffer);
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
    // std::string a;
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
