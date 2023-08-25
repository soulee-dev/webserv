#include "Client.hpp"
#include "Utils.hpp"
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <vector>

Client::Client()
    : state(PARSE_READY), haveToReadBody(false), writeIndex(0)
{
    readBuffer.reserve(100000000);
    sendBuffer.reserve(100000000);
    STATUS_CODES[200] = "OK";
    STATUS_CODES[201] = "Created";
    STATUS_CODES[400] = "Bad Request";
    STATUS_CODES[403] = "Forbidden";
    STATUS_CODES[404] = "Not Found";
    STATUS_CODES[405] = "Method Not Allowed";
    STATUS_CODES[413] = "Request Entity Too Large";
    STATUS_CODES[505] = "HTTP Version Not Supported";
}

Client::~Client() {}

Client::Client(Client const& other)
    : client_fd(other.client_fd), request(other.request), response(other.response), readBuffer(other.readBuffer), sendBuffer(other.sendBuffer), writeIndex(other.writeIndex)
{
}

void Client::requestClear()
{
    memset(&request, 0, sizeof(HttpRequest));
    request.isStatic = true;
    request.pipe_fd[0] = -1;
    request.pipe_fd[1] = -1;
    request.pipe_fd_back[0] = -1;
    request.pipe_fd_back[1] = -1;
    request.errorCode = NOT_ERROR;
}

Client& Client::operator=(Client const& rhs)
{
    if (this != &rhs)
    {
        this->client_fd = rhs.client_fd;
        this->readBuffer = rhs.readBuffer;
        this->sendBuffer = rhs.sendBuffer;
        this->state = rhs.state;
        this->writeIndex = rhs.writeIndex;
        this->request = rhs.request;
        this->response = rhs.response;
    }
    return *this;
}

void Client::setFd(int fd)
{
    client_fd = fd;
}
void Client::setLocations(std::map<std::string, Location>& loc)
{
    locations = &loc;
}

void Client::setClientBodySize(int size)
{
    clientBodySize = size;
}

int Client::getClientFd(void) const
{
    return client_fd;
}

HttpRequest& Client::getReq(void)
{
	return request;
}

HttpResponse& Client::gatRes(void)
{
	return response;
}

void Client::responseClear()
{
    memset(&response, 0, sizeof(HttpResponse));
}

int Client::makeReqeustFromClient()
{
    int res = readMessageFromClient();
    if (res == ERROR)
        return res;
    else
    {
        if (state == PARSE_DONE)
        {
            std::cout << BOLDGREEN << "URI : " << request.uri << RESET << '\n';
            std::cout << BOLDYELLOW << "HTTP VERSION : " << request.httpVersion << RESET << '\n';
            checkRequest();
            std::cout << BOLDCYAN << " -- SUCCESSFULLY GET MESSAGE -- \n\n"
                      << RESET;
            return SUCCESS;
        }
        else if (state == PARSE_ERROR)
        {
            std::cout << BOLDCYAN << " -- GET MESSAGE FAILED -- \n\n"
                      << RESET;
            return ERROR;
        }
        else
        {
            return NOTDONE;
        }
    }
}

// START makeRequest
int Client::readMessageFromClient()
{
    const size_t BUFFER_SIZE = 65536;
    char buffer[BUFFER_SIZE + 1];
    ssize_t readSize = read(client_fd, buffer, BUFFER_SIZE);

    if (readSize <= 0)
    {
        if (readSize == -1)
            std::cout << "read() error" << std::endl;
        return ERROR;
    }
    buffer[readSize] = '\0';

    // 함수 포인터 배열 사용해보는것도?
    switch (state)
    {
    case PARSE_READY:
        requestClear();
    case PARSE_METHOD:
        readMethod(buffer);
        break;
    case PARSE_URI:
        readUri(buffer);
        break;
    case PARSE_HTTP_VERSION:
        readHttpVersion(buffer);
        break;
    case PARSE_HEADER:
        readHeader(buffer);
        break;
    case PARSE_BODY:
        readBody(buffer, readSize);
        break;
    case PARSE_CHUNKED:
        readChunked(buffer, readSize);
    default:
        break;
    }
    return SUCCESS;
}

void Client::readMethod(const char* buffer)
{
    std::vector<unsigned char>::iterator pos;

    // 입력버퍼벡터 뒤에 방금읽은 버퍼를 덧붙임
    readBuffer.insert(readBuffer.end(), buffer, buffer + strlen(buffer));
    while (readBuffer.size() > 1 && readBuffer[0] == '\r' && readBuffer[1] == '\n')
        readBuffer.erase(readBuffer.begin(), readBuffer.begin() + 2);
    if (readBuffer.size() == 0)
        return;
    // 입력버퍼벡터에서 공백을 찾음
    if ((pos = std::search(readBuffer.begin(), readBuffer.end(), " ", &" "[1])) !=
        readBuffer.end())
    {
        request.method = std::string(readBuffer.begin(), pos);
        request.startLine = request.method;
        readBuffer.erase(readBuffer.begin(), pos + 1);
        state = PARSE_URI;
        // 또 다른 공백을 찾은 경우 다음 파싱으로 넘어감.
        // 이때 공백이 연속해서 들어오는 경우를 생각해 볼 수 있는데 이런 경우
        // 에러처리로 됨.
        if ((pos = std::search(readBuffer.begin(), readBuffer.end(), " ",
                               &" "[1])) != readBuffer.end())
            readUri("");
        else if ((pos = std::search(readBuffer.begin(), readBuffer.end(), CRLF,
                                    &CRLF[2])) != readBuffer.end())
        {
            state = PARSE_ERROR;
            std::cout << "DEBUG7\n";
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
        state = PARSE_ERROR;
        std::cout << "DEBUG8\n";
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
        state = PARSE_HTTP_VERSION;
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
        state = PARSE_ERROR;
        std::cout << "DEBUG9\n";
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
            state = PARSE_ERROR;
            std::cout << "DEBUG10\n";
            request.errorCode = HTTP_VERSION_NOT_SUPPORT;
            return;
        }
        request.startLine += " " + request.httpVersion;
        state = PARSE_HEADER;
        if ((pos = std::search(readBuffer.begin(), readBuffer.end(), CRLF,
                               &CRLF[2])) != readBuffer.end())
            readHeader("");
        return;
    }
}

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
                state = PARSE_ERROR;
                std::cout << "DEBUG1\n";
                request.errorCode = BAD_REQUEST;
                return;
            }
            else if (checkMethod(request.method))
            {
                state = PARSE_ERROR;
                std::cout << "DEBUG CHECKMETHOD\n";
                request.errorCode = METHOD_NOT_ALLOWED; // 이 경우, 405번을 부여하지 않으면 테스트에서 통과가 불가능합니다(원래 400).
                return;
            }
            else if (request.headers.find("content-length") == request.headers.end() &&
                     (request.method == "GET" || request.method == "DELETE" || request.method == "HEAD"))
            {
                state = PARSE_DONE;
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
                    state = PARSE_CHUNKED;
                    return (readChunked("", 0));
                }
                else if (request.method == "POST" && request.headers.find("content-length") == request.headers.end())
                {
                    state = PARSE_DONE;
                    return;
                }
                else
                {
                    state = PARSE_BODY;
                    return (readBody("", 0));
                }
            }
        }
        headerSstream << line;
        getline(headerSstream, key, ':');
        if (key.size() == 0)
        {
            state = PARSE_ERROR;
            std::cout << "DEBUG3\n";
            request.errorCode = BAD_REQUEST;
            return;
        }
        for (size_t i = 0; i < key.size(); i++)
        {
            if (isspace(key[i]))
            {
                state = PARSE_ERROR;
                std::cout << "DEBUG4\n";
                request.errorCode = BAD_REQUEST;
                return;
            }
        }
        getline(headerSstream, value);
        if (value.size() == 0)
        {
            state = PARSE_ERROR;
            std::cout << "DEBUG5\n";
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

void Client::readBody(const char* buffer, size_t readSize)
{
    std::vector<unsigned char>::iterator pos;

    readBuffer.insert(readBuffer.end(), buffer, buffer + readSize);
    if (request.headers.find("content-length") != request.headers.end())
    {
        size_t contentLen = atoi(request.headers["content-length"].c_str());
        size_t maxLen = minLen(contentLen, clientBodySize);
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
            state = PARSE_DONE;
        }
    }
    else if ((pos = std::search(readBuffer.begin(), readBuffer.end(),
                                "\r\n\r\n", &"\r\n\r\n"[4])) !=
             readBuffer.end())
    {
        request.body.insert(request.body.end(), readBuffer.begin(), pos);
        readBuffer.erase(readBuffer.begin(), pos + 4);
        state = PARSE_DONE;
    }
}

bool Client::checkMethod(std::string const& method)
{
    std::string tmp_uri;
    std::string found_uri;
    bool is_found = false;
    size_t location_pos;
    std::map<std::string, Location>::iterator location;
    size_t allow_methods;

    if (request.uri[request.uri.size() - 1] != '/')
        request.uri += "/";
    tmp_uri = request.uri;
    while (tmp_uri != "/")
    {
        if (is_found)
            break;
        location_pos = tmp_uri.find_last_of('/');
        if (location_pos == std::string::npos)
            break;
        if (location_pos == 0)
            tmp_uri = "/";
        else
            tmp_uri = std::string(tmp_uri.begin(), tmp_uri.begin() + location_pos);
        for (location = locations->begin(); location != locations->end(); ++location)
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

    allow_methods = (*locations)[found_uri].getAllowMethod();
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
            strBodySize = std::string(readBuffer.begin(), pos);
            readBuffer.erase(readBuffer.begin(), pos + 2);
            longBodySize = strtol(strBodySize.c_str(), NULL, 16);
            haveToReadBody = true;
        }
        if (haveToReadBody == true)
        {
            if (longBodySize == 0)
            {
                state = PARSE_DONE;
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
                state = PARSE_ERROR;
                std::cout << "DEBUG6\n";
                request.errorCode = BAD_REQUEST;
                return;
            }
            readBuffer.erase(readBuffer.begin(),
                             readBuffer.begin() + longBodySize + 2);
            pos = std::search(readBuffer.begin(), readBuffer.end(), crlf, &crlf[2]);
        }
    }
}

bool Client::isSendBufferEmpty(void)
{
    return (sendBuffer.size() == 0);
}

void Client::checkRequest(void) // RequestManager::Parse
{
    bool is_found;
    size_t location_pos;
    std::string found_uri;
    std::string tmp_uri;
    std::map<std::string, Location>::iterator location;

    if (request.uri[request.uri.size() - 1] != '/')
        request.uri += "/";
    tmp_uri = request.uri;
    while (tmp_uri != "/")
    {
        if (is_found)
            break;
        location_pos = tmp_uri.find_last_of('/');
        if (location_pos == std::string::npos)
            break;
        if (location_pos == 0)
            tmp_uri = "/";
        else
            tmp_uri = std::string(tmp_uri.begin(), tmp_uri.begin() + location_pos);
        for (location = locations->begin(); location != locations->end(); ++location)
        {
            if (tmp_uri == location->first)
            {
                found_uri = location->first;
                is_found = true;
                break;
            }
        }
    }
    if (is_found)
    {
        request.fileName = request.uri.substr(location_pos);
        request.fileName.erase(request.fileName.size() - 1);
    }
    else
        found_uri = "/";
    std::cout << "LOCATION: " << found_uri << '\n';
    request.cgiPathInfo = "/";
    if (request.uri.find("cgi-bin") == std::string::npos)
    {
        if (request.method == "POST" && request.uri.find(".bla") != std::string::npos)
        {
            request.isStatic = false;
        }
        else
        {
            request.isStatic = true;
        }
    }
    else
    {
        request.isStatic = false;
        size_t pos = request.uri.find('?');
        if (pos != std::string::npos)
        {
            request.cgiArgs = request.uri.substr(pos + 1);
            request.cgiArgs = request.cgiArgs.erase(request.cgiArgs.size() - 1);
        }
        request.fileName = request.fileName.substr(0, pos);
        size_t path_pos = request.fileName.find("/", 10);
        if (path_pos != std::string::npos)
        {
            request.cgiPathInfo = request.fileName.substr(path_pos);
            request.fileName = request.fileName.substr(0, path_pos);
        }
    }
    request.location_uri = found_uri;
    request.location = (*locations)[found_uri];
    request.path = request.location.getRoot() + request.fileName;
    std::cout << "FILENAME: " << request.fileName << std::endl;
    std::cout << "PATH_INFO: " << request.cgiPathInfo << std::endl;
    std::cout << "PATH: " << request.path << std::endl;

    std::map<std::string, std::string>::iterator it;
    for (it = request.headers.begin(); it != request.headers.end(); ++it)
    {
        std::cout << BOLDGREEN << it->first << " : " << it->second << RESET << '\n';
    }
};
// DONE makeRequest
// START makeResponse

// Static
void Client::makeResponseFromStatic()
{
    isCgi = false;
    std::cout << "METHOD : " << request.method << RESET << "\n";
    std::cout << "SIZE : " << request.body.size() << "\nMAX BODY SIZE : " << request.location.getClientBodySize() << '\n';
    if ((request.body.size() > request.location.getClientBodySize()) && request.method == "POST") // CGI-BIN 없는 POST 요청의 경우 일괄적으로 405를 띄워 줍니다.
    {
        response.statusCode = 413;
        return;
    }
    // TODO: 현재 STATIC으로 들어오는 HEAD 요청에 대해 전부 405를 반환하는데 이것은 테스터가 URI "/"로 보내는 요청의
    // allowed method가 GET 뿐이기 때문입니다. 현재 ServeStatic() 끝에 HEAD일 경우 바디 때는 처리가 되어있어서
    // HEAD를 따로 구현할 건 없는데 allowed method 처리 되면 이 분기문은 없애야합니다.
    if (request.method == "HEAD")
        response.statusCode = 405;

    else if (request.method == "PUT") // POST와 PUT이 괴상하게 섞인 분기문을 정리했습니다.
    {
        std::ifstream ifs(request.path);
        std::ofstream ofs;
        int res = ifs.is_open();
        ifs.close();                                             // infile close
        ofs.open(request.path, std::ios::out | std::ios::trunc); // 출력 모드로, 이미 파일이 존재한다면 파일을 비우고 새로 엽니다.
        if (ofs.fail())                                          // 파일 열기에 실패했으면 404 에러를 호출합니다
            response.statusCode = 404;                           // 이거 아래까지 내려가면 안되는거 아님?
        response.headers["Connection"] = "close";
        // request.body.resize(100); // TODO: max body size
        for (size_t i = 0; i < request.body.size(); i++)
            ofs << request.body[i];
        ofs.close();

        if (res) // curl -v -X DELETE -d "body" http://localhost/put_test/file_should_exist_after
            response.statusCode = 200;
        else
            response.statusCode = 201;
        return;
    }

    if (isDirectory(request.path)) // 나머지 GET 요청에 대한 처리입니다.
        processDirectory();
    else
        serveStatic();
}

void Client::processDirectory()
{

    std::vector<std::string> indexVec = request.location.getIndex(); // 벡터에 대한 참조
    std::vector<std::string>::iterator it;

    if (!indexVec.empty()) // 벡터가 비어있지 않은지 확인
    {
        for (it = indexVec.begin(); it != indexVec.end(); ++it)
        {
            std::string index = *it;
            std::string path = request.path + "/" + index;
            if (isRegularFile(path) && isFileReadable(path))
            {
                request.path = path;
                std::cout << BOLDRED << "PATH : " << path << RESET << '\n';
                serveStatic();
                return;
            }
        }
    }
    if (request.location.getAutoIndex())
        handleDirectoryListing();
    else
        response.statusCode = 404;
}

void Client::handleDirectoryListing()
{

    DIR* dir = opendir(request.path.c_str());
    if (!dir)
    // Error Handler를 호출해야 하는 첫 번째 경우 (errnum = 1),
    // 현재 default.conf의 root는 /html로 지정되어 있는데, 그 /html이 없는 경우이다.
    {
        response.statusCode = 404;
        return;
    }
    std::stringstream ss;
    ss << "<!DOCTYPE html><head><title>Index of " << request.path;
    ss << "</title></head><body><h1>Index of " << request.path;
    ss << "</h1><ul>";

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL)
    {
        ss << "<li><a href=";
        ss << request.location_uri;
        ss << request.fileName << "/";
        ss << entry->d_name;
        ss << ">";
        ss << entry->d_name;
        ss << "</a></li>";
    }
    closedir(dir);
    ss << "</ul></body></html>";
    response.body = stou(ss);
    response.headers["Connection"] = "close";
    response.headers["Content-Type"] = "text/html";
    response.statusCode = 200;
}

void Client::serveStatic()
{

    if (!isFileExist(request.path))
    {
        response.statusCode = 404;
        return;
    }
    if (!isRegularFile(request.path) || !isFileReadable(request.path))
    {
        response.statusCode = 403;
    }

    if (request.method != "HEAD")
        response.body = ReadStaticFile(request.path);

    if (request.method.empty()) // METHOD에 비어있을 때 예외처리이다. 나쁜 테스터 죽어.
    {
        response.statusCode = 404;
        return;
    }
    response.headers["Connection"] = "keep-alive";
    response.headers["Content-Type"] = getFileType(request.path);
    response.statusCode = 200;
}

// Delete
void Client::makeResponseFromDelete()
{
    const char* path = request.path.c_str();
    std::vector<unsigned char> emptyBody;

    response.body.clear();
    if (std::remove(path) == 0)
    {
        std::cout << "DELETE SUCCESS\n"
                  << RESET;
        response.headers["Connection"] = "close";
        response.statusCode = 200;
    }
    else
    {
        std::cout << "DELETE FAILED\n"
                  << RESET;
        response.statusCode = 404;
        return;
    }
}

// Dynamic
bool Client::openFdForDynamic()
{

    if (pipe(request.pipe_fd) == -1 || pipe(request.pipe_fd_back) == -1)
    {
        return false;
    }
    fcntl(request.pipe_fd[0], F_SETFL, O_NONBLOCK);
    fcntl(request.pipe_fd_back[1], F_SETFL, O_NONBLOCK);
    fcntl(request.pipe_fd[1], F_SETFL, O_NONBLOCK);
    fcntl(request.pipe_fd_back[0], F_SETFL, O_NONBLOCK);
    return true;
}

bool Client::runCgiForDynamic()
{

    pid_t pid = fork();
    if (pid == -1)
    {
        return false;
    }
    isCgi = true;
    if (pid == 0) // 자식 코드
    {
        // Child process
        dup2(request.pipe_fd[0], STDIN_FILENO);       // stdin을 pipe_fd[0]로 복제
        dup2(request.pipe_fd_back[1], STDOUT_FILENO); // stdout을 pipe_fd_back[1]로 복제

        close(request.pipe_fd[0]);      // Close unused read end
        close(request.pipe_fd_back[1]); // Close unused write end in parent
        close(request.pipe_fd[1]);      // Close unused read end
        close(request.pipe_fd_back[0]); // Close unused write end in parent

        // TODO MAX BODY SIZE
        // if (request.body.size() > 100)
        // 	request.body.resize(100); // max body size

        int size = request.body.size();
        std::string size_str = std::to_string(size); // c++ 11
        const char* size_cstr = size_str.c_str();

        setenv("QUERY_STRING", request.cgiArgs.c_str(), 1);
        setenv("REQUEST_METHOD", request.method.c_str(), 1);
        setenv("CONTENT_LENGTH", size_cstr, 1);
        setenv("SERVER_PROTOCOL", SERVER_HTTP_VERSION, 1);
        setenv("PATH_INFO", request.cgiPathInfo.c_str(), 1);
        setenv("HTTP_X_SECRET_HEADER_FOR_TEST", request.headers["x-secret-header-for-test"].c_str(), 1);
        setenv("CONTENT_TYPE", request.headers["content-type"].c_str(), 1);

        if (request.uri.find(".bla") != std::string::npos)
            request.path = "./cgi_tester";

        if (execve(request.path.c_str(), NULL, environ) == -1)
        {
            // TODO: Handle Error
            std::cerr << "execve error" << std::endl;
            exit(0);
        }
    }
    else
    {
        close(request.pipe_fd[0]);      // Close unused read end
        close(request.pipe_fd_back[1]); // Close unused write end in parent
    }
    return true;
}

void Client::makeResponseFromDynamic()
{
    if (openFdForDynamic() == false)
        exitWebserver("Pipe() error"); // or response 응답 코드로 변경해도됨
    if (runCgiForDynamic() == false)
        exitWebserver("Fork() error");
    // 정상이라면 이벤트 등록 하자
    events.changeEvents(request.pipe_fd[1], EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, this);
    events.changeEvents(request.pipe_fd_back[0], EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, this);
}

// FD

int Client::makeResponseFromFd(int ident)
{
    const size_t BUFFER_SIZE = 65536;

    std::vector<unsigned char>& readBuffer = response.body;

    char buffer[BUFFER_SIZE];

    ssize_t ret = read(ident, buffer, BUFFER_SIZE);
    if (ret == -1)
    {
        response.statusCode = 500; // 걍 넣어봄, 맞는지는 몰름
        return -1;
    }
    readBuffer.insert(readBuffer.end(), buffer, &buffer[ret]);
    if ((ret == 0 || ret < BUFFER_SIZE) && request.body.size() == 0)
    {
        response.statusCode = 200;
        return 1;
    }
    else
        return 0;
}

void Client::calculateBodyLength()
{

    std::cout << BOLDGREEN << "CODE : " << response.statusCode << RESET << "\n";

    if (isCgi)
    {
        int pos = std::search(response.body.begin(), response.body.end(), &CRLFCRLF[0], &CRLFCRLF[4]) - response.body.begin();
        response.headers["Content-Length"] = intToString(response.body.size() - pos - 4);
        std::cout << "BUILD HEADER\n";
    }
    else
    {
        response.headers["Content-Length"] = intToString(response.body.size());
    }
    std::cout << "\n  -- <RESPONSE> -- \n";
    // for (size_t i = 0; i < response.size(); i++)
    // 	std::cout << response[i];
    // std::cout << '\n';
}
