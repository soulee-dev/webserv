#pragma once
#include "Color.hpp"
#include "Event.hpp"
#include "Http/Handler/Handler.hpp"
#include "Location.hpp"
#define SUCCESS 1
#define ERROR -1
#define NOTDONE 0

#define SPACE " "
#define CRLF "\r\n"
#define CRLFCRLF "\r\n\r\n"
#define SERVER_NAME "Master J&J Server"
#define SERVER_HTTP_VERSION "HTTP/1.1"
#define COLON ":"

extern char** environ;
extern Event events;
enum State
{
    PARSE_READY,
    PARSE_METHOD,
    PARSE_URI,
    PARSE_HTTP_VERSION,
    PARSE_HEADER,
    PARSE_BODY,
    PARSE_DONE,
    PARSE_CHUNKED,
    PARSE_ERROR,
    //
    // MAKE_CGI,
    // MAKE_PUT,

    SEND_STARTLINE,
    SEND_HEADERS,
    SEND_BODY,
    DONE,
};

enum ParseErrorCode
{
    NOT_ERROR = 0,
    BAD_REQUEST = 400,
    METHOD_NOT_ALLOWED = 405,
    HTTP_VERSION_NOT_SUPPORT = 505,
};

enum METHOD
{
    GET = 1,
    POST = 2,
    PUT = 4,
    DELETE = 8,
    HEAD = 16,
};

// HTTP REQ and RES
struct HttpRequest
{
    int writeIndex;
    std::string startLine;
    std::string httpVersion;
    bool isStatic;
    int fileFd;
	int file_size;
	int RW_file_size;
    std::string fileName;
    std::string path;
    std::string cgiArgs;
    std::string cgiPathInfo;
    int pipe_fd[2];
    int pipe_fd_back[2];
    Location location;
    std::string location_uri;
    std::string uri;
    enum ParseErrorCode errorCode;
    std::string method;
    std::string header;
    std::map<std::string, std::string> headers;
    std::vector<unsigned char> body;
};

struct HttpResponse
{
    int statusCode;
    std::string startLine;
    std::string httpVersion;
    std::map<std::string, std::string> headers;
    std::vector<unsigned char> body;
};

class Client
{
private:
    int client_fd;
    int state; // Parsing + Res
    int isCgi;
    std::map<int, std::string> STATUS_CODES;

    std::map<std::string, Location>* locations;
    HttpRequest request;
    HttpResponse response;

    std::vector<unsigned char> readBuffer;
    std::vector<unsigned char> sendBuffer;
    int writeIndex;

    int haveToReadBody;
    std::string strBodySize;
    long longBodySize;
    int clientBodySize;

public:
    typedef int PORT;
    typedef int SOCKET;
    Client();
    Client(const Client& ref);
    Client& operator=(const Client& ref);

    ~Client();

    // setter
    void setFd(int fd);
    void setLocations(std::map<std::string, Location>& loc);
    void setClientBodySize(int size);
    // getter
    int getClientFd(void) const;
    HttpRequest& getReq(void);
    HttpResponse& getRes(void);

    // functions
    void requestClear();
    void responseClear();

    int makeReqeustFromClient();
    int readMessageFromClient();

    void makeResponseFromStatic();
    void processDirectory();
    void serveStatic();
    void handleDirectoryListing();

    void makeResponseFromDelete();
    void makeResponseFromDynamic();

    int makeResponseFromFd(int ident);
    void calculateBodyLength();

    void buildHeader();
    void makeSendBufferForWrite();
    int writeSendBufferToClient();
	int writeRequestBodyToFd(int fd);
    // int sendResponseToClient();

private:
    void readMethod(const char* buffer);
    void readUri(const char* buffer);
    void readHttpVersion(const char* buffer);
    void readHeader(const char* buffer);
    void readBody(const char* buffer, size_t readSize);
    void readChunked(const char* buffer, size_t readSize);
    bool isSendBufferEmpty(void);
    bool checkMethod(std::string const& method);
    void checkRequest(void);
    bool openFdForDynamic();
    bool runCgiForDynamic();
};
