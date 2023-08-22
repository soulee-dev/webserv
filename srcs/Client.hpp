#pragma once
#include "Event.hpp"
#include "Location.hpp"
#include "ResponseMessage.hpp"
#include "Server.hpp"
#include "Http/HttpRequestManager.hpp"
#include <queue>

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
    // std::queue<ResponseMessage> queRes; // 가져갈땐 pop, 넣을땐 push
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
    ResponseMessage res;
    HttpRequest req;
    Event* events;
    HttpRequestManager httpRequestManager;
	std::vector<unsigned char> sendBuffer;
    int         writeIndex;
    typedef int PORT;
    typedef int SOCKET;
    Client();
    Client(const Client& ref);
    Client& operator=(const Client& ref);

    ~Client();

    // setter
    void setFd(int fd);
    void setServer(Server* server);
    void setEvents(Event* event);

    // getter
    Server* getServer(void) const;
    SOCKET getClientFd(void) const;

    // functions
    void errorEventProcess(void);
    bool readEventProcess(void);
    bool writeEventProcess(void);
    bool readMessage(void);
    bool checkMethod(std::string const& method);
    bool isSendBufferEmpty(void);
    void clearRes(void);
    void clearReq(void);
};
