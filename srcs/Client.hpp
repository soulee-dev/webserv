#pragma once

#include "Event.hpp"
#include "Location.hpp"
#include "Message/Response.hpp"
#include "Server.hpp"
#include "Http/HttpRequestManager.hpp"

enum RequestMessageParseState
{
    READY,
    METHOD,
    URI,
    HTTP_VERSION,
    HEADER,
    BODY,
    DONE,
    CHUNKED,
    ERROR,
};

class Client
{
private:
    int client_fd;
    Server* server;
    // event 등록;
	std::vector<unsigned char> readBuffer;
	RequestMessageParseState parseState;

	std::string strbodySize;
	long longBodySize;
	bool haveToReadBody;

    void readMethod(const char* buffer);
    void readUri(const char* buffer);
    void readHttpVersion(const char* buffer);
    void readHeader(const char* buffer);
    void readBody(const char* buffer, size_t readSize);
    void readChunked(const char* buffer, size_t readSize);

public:
    Request request;
    Response response;
    Event* events;
    HttpRequestManager httpRequestManager;
	std::vector<unsigned char> sendBuffer;
    int         writeIndex;
    typedef int PORT;
    typedef int SOCKET;
    Client();
    ~Client();

    void setFd(int fd);
    void setServer(Server* server);
    void setEvents(Event* event);

    Server* getServer(void) const;
    SOCKET getClientFd(void) const;

    void errorEventProcess(void);
    bool readEventProcess(void);
    bool writeEventProcess(void);
    bool readMessage(void);
    bool checkMethod(std::string const& method);
    bool isSendBufferEmpty(void);
};
