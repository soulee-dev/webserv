#pragma once
#include "Location.hpp"
#include "RequestMessage.hpp"
#include "ResponseMessage.hpp"
#include "Server.hpp"
#include "Event.hpp"

enum RequestMessageParseState
{
	METHOD,
	REQUEST_TARGET,
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
    Event* events;

    RequestMessage req;
    ResponseMessage res;

    std::vector<unsigned char> readBuffer;
    std::vector<unsigned char> sendBuffer;
    RequestMessageParseState parseState;

	void readMethod(const char *buffer);
	void readUri(const char *buffer);
	void readHttpVersion(const char *buffer);
	void readHeader(const char *buffer);
	void readBody(const char *buffer, size_t readSize);
	void readChunked(const char *buffer, size_t readSize);

public:
    typedef int PORT;
    typedef int SOCKET;
    Client();
    Client(const Client& ref);
    Client& operator=(const Client& ref);

    ~Client();
    void runServer(void);


    // setter
    void setFd(int fd);
    void setServer(Server* server);
    void setEvents(Event* event);

    // getter
    ResponseMessage& getRes(void);
    RequestMessage& getReq(void);
    Server* getServer(void) const;
    SOCKET getClientFd(void) const;

    // functions
    void errorEventProcess(void);
    bool readEventProcess(void);
    bool writeEventProcess(void);

    bool readMessage(void);
    bool isSendBufferEmpty(void);
};
