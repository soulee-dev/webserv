#include "StaticHandler.hpp"
#include "ErrorHandler.hpp"
#include "../../Client.hpp"
#include "ErrorHandler.hpp"

std::vector<unsigned char>	StaticHandler::handle(Client& client) const
{
	Request&				request = client.request;
	std::vector<unsigned char>	result;

	std::cout << "METHOD : " << request.method << RESET << '\n';

	std::cout << "SIZE : " << request.body.size() << "\nMAX BODY SIZE : " << request.location.getClientBodySize() << '\n';
	if ((request.body.size() > request.location.getClientBodySize()) && request.method == "POST")
	{
		return ErrorHandler::handle(client, 413);
	}
	if (request.method == "HEAD")
		return ErrorHandler::handle(client, 405);

	else if (request.method == "PUT")
	{
		std::ifstream	ifs(request.path);
		std::ofstream	ofs;
		int res = ifs.is_open();
		ifs.close(); // infile close
		ofs.open(request.path, std::ios::out | std::ios::trunc);
		std::map<std::string, std::string>  headers;
		if (ofs.fail())
			ErrorHandler::handle(client, 404);
		headers["Connection"] = "close";
		for (size_t i = 0; i < request.body.size(); i++)
			ofs << request.body[i];
		ofs.close();

		std::vector<unsigned char> empty_body;
		if (res)
			return BuildResponse(200, headers, empty_body);
		return BuildResponse(201, headers, empty_body);
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

std::vector<unsigned char>	StaticHandler::HandleDirectoryListing(Client& client, Request& request) const
{
	std::vector<unsigned char>			body;
	std::map<std::string, std::string>	headers;

	DIR	*dir = opendir(request.path.c_str());
	if (!dir)
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
    Request& request = client.request;
    std::vector<std::string> indexVec = request.location.getIndex(); // 벡터에 대한 참조
    std::vector<std::string>::iterator it;

    if (!indexVec.empty())
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
