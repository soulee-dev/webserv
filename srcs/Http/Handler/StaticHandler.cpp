#include "StaticHandler.hpp"
#include "ErrorHandler.hpp"
#include "../../Client.hpp"
#include "ErrorHandler.hpp"

std::vector<unsigned char>	StaticHandler::handle(Client& client) const
{
	HttpRequest&				request = client.httpRequestManager.getRequest();
	std::vector<unsigned char>	result;

	if (request.method == "POST")
		return ErrorHandler::handler(405);
    if (IsDirectory(request.path))
        return ProcessDirectory(client);
	return ServeStatic(request.path);
}

int	is_directory(std::string fileName)
{
	if (fileName.back() == '/')
		return 1;
	return (0);
}

std::vector<unsigned char>	StaticHandler::HandleDirectoryListing(HttpRequest& request) const
{	
	std::vector<unsigned char>			body;
	std::map<std::string, std::string>	headers;

	DIR	*dir = opendir(request.path.c_str());
	if (!dir) 
	// Error Handler를 호출해야 하는 첫 번째 경우 (errnum = 1), 
	// 현재 default.conf의 root는 /html로 지정되어 있는데, 그 /html이 없는 경우이다.
		return ErrorHandler::handler(404);

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
                return ServeStatic(request.path);
            }
        }
    }
    if (request.location.getAutoIndex())
        return HandleDirectoryListing(request);
    return ErrorHandler::handler(404);
}

StaticHandler::~StaticHandler()
{}

void StaticHandler::sendReqtoEvent(Client &client)
{
	client.sendBuffer = handle(client);
	client.events->changeEvents(client.getClientFd(), EVFILT_WRITE, EV_ENABLE | EV_ADD, 0, 0, &client);
}
