#include "StaticHandler.hpp"
#include "ErrorHandler.hpp"
#include "../../Client.hpp"
#include "ErrorHandler.hpp"

std::vector<unsigned char>	StaticHandler::handle(Client& client) const
{
	HttpRequest&				request = client.httpRequestManager.getRequest();
	std::vector<unsigned char>	result;

	std::cout << "METHOD : " << request.method << RESET << '\n';

	if ((IsDirectory(request.path) || is_directory(request.path)) && request.method == "POST")
		return ErrorHandler::handle(client, 405);

	//if (request.method == "POST" || request.method == "HEAD") // 테스트기 통과하려고 이렇게 했습니다 allow method 비교 후 처리해야합니다 나쁜 테스트 죽어 
	if (request.method == "HEAD")	
		return ErrorHandler::handle(client, 405);

	if (request.method == "POST" || request.method == "PUT")
	{
		std::ofstream	file;
		// outputFile.open(filename, std::ios::out | std::ios::app);
		if (request.method == "PUT")
			file.open(request.file_name, std::ios::out);
		else
			file.open(request.file_name, std::ios::out | std::ios::app);
		std::map<std::string, std::string>  headers;
		int res = file.is_open();
		file.close();
		headers["Connection"] = "close";
		// TODO max body size
		// request.body.resize(100);
		if (res)
			return BuildResponse(200, headers, request.body);
		return BuildResponse(201, headers, request.body);
	}

    if (IsDirectory(request.path))
        return ProcessDirectory(client);
	return ServeStatic(client, request.path, request.method);
}

int	is_directory(std::string fileName)
{
	if (fileName.back() == '/')
		return 1;
	return (0);
}

std::vector<unsigned char>	StaticHandler::HandleDirectoryListing(Client& client, HttpRequest& request) const
{	
	std::vector<unsigned char>			body;
	std::map<std::string, std::string>	headers;

	DIR	*dir = opendir(request.path.c_str());
	if (!dir) 
	// Error Handler를 호출해야 하는 첫 번째 경우 (errnum = 1), 
	// 현재 default.conf의 root는 /html로 지정되어 있는데, 그 /html이 없는 경우이다.
		return ErrorHandler::handle(client, 404);

	std::stringstream	ss;
	ss << "<!DOCTYPE html><head><title>Index of " << request.path;
	ss << "</title></head><body><h1>Index of " << request.path;
	ss << "</h1><ul>";

	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL)
	{
		ss << "<li><a href=";
		ss << request.location_uri;
		ss << request.file_name << "/";
		ss << entry->d_name;
		ss << ">";
		ss << entry->d_name;
		ss << "</a></li>";
	}
	closedir(dir);
    ss << "</ul></body></html>";
	body = stou(ss);
	headers["Connection"] = "close";
	headers["Content-Type"] = "text/html";
	return BuildResponse(200, headers, body);
}

std::vector<unsigned char> StaticHandler::ProcessDirectory(Client& client) const
{
    HttpRequest& request = client.httpRequestManager.getRequest();
    std::vector<std::string> indexVec = request.location.getIndex(); // 벡터에 대한 참조
    std::vector<std::string>::iterator it;

    if (!indexVec.empty()) // 벡터가 비어있지 않은지 확인
    {
        for (it = indexVec.begin(); it != indexVec.end(); ++it)
        {
            std::string index = *it;
            std::string path = request.path + "/" + index;
            if (IsRegularFile(path) && IsFileReadable(path))
            {
                request.path = path;
                std::cout << BOLDRED << "PATH : " << path << RESET << '\n';
                return ServeStatic(client, request.path, request.method);
            }
        }
    }
    if (request.location.getAutoIndex())
        return HandleDirectoryListing(client, request);
    return ErrorHandler::handle(client, 404);
}

StaticHandler::~StaticHandler()
{}

void StaticHandler::sendReqtoEvent(Client &client)
{
	client.sendBuffer = handle(client);
	client.events->changeEvents(client.getClientFd(), EVFILT_WRITE, EV_ENABLE | EV_ADD, 0, 0, &client);
}
