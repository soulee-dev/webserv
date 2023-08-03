#include "RequestMessageReader.hpp"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include "Color.hpp"

RequestMessageReader& RequestMessageReader::getInstance()
{
    static RequestMessageReader instance;
    return instance;
}

void RequestMessageReader::insertNewClient(int client_fd)
{
    this->messageBuffer[client_fd] = RequestMessage();
    this->readBuffer[client_fd] = std::vector<unsigned char>();
    this->ParseState[client_fd] = METHOD;
}

void RequestMessageReader::deleteClient(int client_fd)
{
    this->messageBuffer.erase(client_fd);
    this->readBuffer.erase(client_fd);
    this->ParseState.erase(client_fd);
}

bool RequestMessageReader::readMessage(int client_fd)
{
    const size_t BUFFER_SIZE = 1024;
    char buffer[BUFFER_SIZE + 1];
    int readSize = read(client_fd, buffer, BUFFER_SIZE);
    std::cout << BOLDRED << "read size : " << readSize << RESET << '\n';
    if (readSize <= 0)
    {
        if (readSize == -1)
            std::cout << "read() error" << std::endl;
        return true;
    }
    buffer[readSize] = '\0';

    switch (ParseState[client_fd])
    {
    case METHOD:
        readMethod(buffer, client_fd);
        break;
    case REQUEST_TARGET:
        readRequestTarget(buffer, client_fd);
        break;
    case HTTP_VERSION:
        readHttpVersion(buffer, client_fd);
        break;
    case HEADER:
        readHeader(buffer, client_fd);
        break;
    case BODY:
        readBody(buffer, client_fd);
        break;
    default:
        break;
    }
    return false;
}

void RequestMessageReader::readHeader(const char* buffer, int client_fd)
{
    std::stringstream headerSstream;
    std::string line;
    std::string key;
    std::string value;
    RequestMessage& currMessage = messageBuffer[client_fd];
    std::vector<unsigned char>& currReadBuffer = readBuffer[client_fd];
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
                return;
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
            return;
        }
        for (size_t i = 0; i < key.size(); i++)
        {
            if (isspace(key[i]))
            {
                ParseState[client_fd] = ERROR;
                return;
            }
        }
        getline(headerSstream, value);
        if (value.size() == 0)
        {
            ParseState[client_fd] = ERROR;
            return;
        }
        headerSstream.clear();
        // value의 처음 ifs들을 지움 근데 하나가 아닐 수 있음
        while (value[0] == ' ')
            value.erase(value.begin());
        for (size_t i = 0; i < key.size(); i++)

            key[i] = tolower(key[i]);
        currMessage.headers[key] = value;
    }
}

#include <iostream>

void RequestMessageReader::readBody(const char* buffer, int client_fd)
{
    RequestMessage& currMessage = messageBuffer[client_fd];
    std::vector<unsigned char>& currReadBuffer = readBuffer[client_fd];
    std::vector<unsigned char>::iterator pos;

    currReadBuffer.insert(currReadBuffer.end(), buffer, buffer + strlen(buffer));
    if (currMessage.headers.find("content-length") != currMessage.headers.end())
    {
        size_t lengthToRead = atoi(currMessage.headers["content-length"].c_str()) - currMessage.body.size();
        if (lengthToRead > currReadBuffer.size())
        {
            currMessage.body.insert(currMessage.body.end(), currReadBuffer.begin(), currReadBuffer.end());
            currReadBuffer.clear();
        }
        else
        {
            currMessage.body.insert(currMessage.body.end(), currReadBuffer.begin(), currReadBuffer.begin() + lengthToRead);
            currReadBuffer.erase(currReadBuffer.begin(), currReadBuffer.begin() + lengthToRead);
            ParseState[client_fd] = DONE;
        }
    }
    else if ((pos = std::search(currReadBuffer.begin(), currReadBuffer.end(), "\r\n\r\n", &"\r\n\r\n"[4])) != currReadBuffer.end())
    {
        currMessage.body.insert(currMessage.body.end(), currReadBuffer.begin(), pos);
        currReadBuffer.erase(currReadBuffer.begin(), pos + 4);
        ParseState[client_fd] = DONE;
    }
    // else
    //     ParseState[client_fd] = DONE;
}
void RequestMessageReader::readMethod(const char* buffer, int client_fd)
{
    RequestMessage& currMessage = messageBuffer[client_fd];
    std::vector<unsigned char>& currReadBuffer = readBuffer[client_fd];
    std::vector<unsigned char>::iterator pos;

    // 첫줄에 개행만 들어온 경우 무시
    if (strcmp(buffer, "\n") == 0 || strcmp(buffer, "\r\n") == 0)
        return;
    // 입력버퍼벡터 뒤에 방금읽은 버퍼를 덧붙임
    currReadBuffer.insert(currReadBuffer.end(), buffer, buffer + strlen(buffer));
    // 입력버퍼벡터에서 공백을 찾음
    if ((pos = std::search(currReadBuffer.begin(), currReadBuffer.end(), " ", &" "[1])) != currReadBuffer.end())
    {
        currMessage.method = std::string(currReadBuffer.begin(), pos);
        currMessage.startLine = currMessage.method;
        currReadBuffer.erase(currReadBuffer.begin(), pos + 1);
        ParseState[client_fd] = REQUEST_TARGET;
        // 또 다른 공백을 찾은 경우 다음 파싱으로 넘어감.
        // 이때 공백이 연속해서 들어오는 경우를 생각해 볼 수 있는데 이런 경우 에러처리로 됨.
        if ((pos = std::search(currReadBuffer.begin(), currReadBuffer.end(), " ", &" "[1])) != currReadBuffer.end())
            readRequestTarget("", client_fd);
        else if ((pos = std::search(currReadBuffer.begin(), currReadBuffer.end(), "\r\n", &"\r\n"[2])) != currReadBuffer.end())
        {
            ParseState[client_fd] = ERROR;
        }
    }
    // 공백이 아닌 개행을 읽은 경우 파싱이 완료되지 못하기 때문에 에러
    else if ((pos = std::search(currReadBuffer.begin(), currReadBuffer.end(), "\r\n", &"\r\n"[2])) != currReadBuffer.end())
    {
        ParseState[client_fd] = ERROR;
        currReadBuffer.erase(currReadBuffer.begin(), pos + 2);
    }
}

void RequestMessageReader::readRequestTarget(const char* buffer, int client_fd)
{
    RequestMessage& currMessage = messageBuffer[client_fd];
    std::vector<unsigned char>& currReadBuffer = readBuffer[client_fd];
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

void RequestMessageReader::readHttpVersion(const char* buffer, int client_fd)
{
    RequestMessage& currMessage = messageBuffer[client_fd];
    std::vector<unsigned char>& currReadBuffer = readBuffer[client_fd];
    std::vector<unsigned char>::iterator pos;

    currReadBuffer.insert(currReadBuffer.end(), buffer, buffer + strlen(buffer));
    if ((pos = std::search(currReadBuffer.begin(), currReadBuffer.end(), "\r\n", &"\r\n"[2])) != currReadBuffer.end())
    {
        currMessage.httpVersion = std::string(currReadBuffer.begin(), pos);
        currReadBuffer.erase(currReadBuffer.begin(), pos + 2);
        if (currMessage.httpVersion != "HTTP/1.1" && currMessage.httpVersion != "HTTP/1.0")
        {
            ParseState[client_fd] = ERROR;
            return;
        }
        currMessage.startLine += " " + currMessage.httpVersion;
        ParseState[client_fd] = HEADER;
        if ((pos = std::search(currReadBuffer.begin(), currReadBuffer.end(), "\r\n", &"\r\n"[2])) != currReadBuffer.end())
            readHeader("", client_fd);
        return;
    }
}
