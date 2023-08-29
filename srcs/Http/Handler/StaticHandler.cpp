#include "StaticHandler.hpp"
#include "ErrorHandler.hpp"
#include "../../Client.hpp"
#include "Handler.hpp"

void	HandleStatic(Client& client)
{
	Request&	request = client.request;
	std::vector<unsigned char>	result;

	if ((request.body.size() > request.location.getClientBodySize()) && request.method == "POST")
		return HandleError(client, 413);
	else if (request.method == "HEAD")
		return HandleError(client, 405);
	else if (request.method == "PUT")
	{
		struct stat file_stat;

		client.request.file_fd = open(request.path.c_str(), O_WRONLY | O_CREAT, 0644);
		if (errno == EEXIST)
			client.response.status_code = 201;
		else
			client.response.status_code = 200;
		fcntl(client.request.file_fd, F_SETFL, O_NONBLOCK, FD_CLOEXEC);
		fstat(client.request.file_fd, &file_stat);
		client.request.file_size = file_stat.st_size;
		return ;
	}

    if (IsDirectory(request.path))
        return ProcessDirectory(client);
	return ServeStatic(client, request.path);
}

int	is_directory(std::string fileName)
{
	if (fileName.back() == '/')
		return 1;
	return (0);
}

void	HandleDirectoryListing(Client& client, Request& request)
{
	std::string							path;
	std::vector<unsigned char>			body;
	std::map<std::string, std::string>	headers;

	DIR	*dir = opendir(request.path.c_str());
	if (!dir)
		return HandleError(client, 404);

	if (request.location_uri != "/")
		path = request.location_uri;
	path += request.file_name;
	std::stringstream	ss;
	ss << "<!DOCTYPE html><head><title>Index of " << path;
	ss << "</title></head><body><h1>Index of " << path;
	ss << "</h1><ul>";

	struct dirent* entry;
	std::cout << "serverName : " << client.getServer()->getServerName() << std::endl;
	while ((entry = readdir(dir)) != NULL)
	{
		ss << "<li><a href=";
		ss << path;
		ss << "/";
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
	return SetResponse(client, 200, headers, body);
}

void	ProcessDirectory(Client& client)
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
                return ServeStatic(client, request.path);
            }
        }
    }
	if (request.location.getAutoIndex())
	{
		client.response.is_auto_index = true;
        return HandleDirectoryListing(client, request);
	}
    return HandleError(client, 404);
}
