#include "StaticHandler.hpp"
#include "ErrorHandler.hpp"
#include "../../Client.hpp"

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

		client.request.file_fd = open(request.path.c_str(), O_RDONLY | O_CREAT, 0644);
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
	std::vector<unsigned char>			body;
	std::map<std::string, std::string>	headers;

	DIR	*dir = opendir(request.path.c_str());
	if (!dir)
		return HandleError(client, 404);

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
                return ServeStatic(client, request.path);
            }
        }
    }
    if (request.location.getAutoIndex())
        return HandleDirectoryListing(client, request);
    return HandleError(client, 404);
}
