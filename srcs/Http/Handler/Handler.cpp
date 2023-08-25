#include "Handler.hpp"
#include "HttpStatusCodes.hpp"
#include "../../Client.hpp"
#include "ErrorHandler.hpp"
#include <algorithm>
#include <vector>

std::string	GetFileType(std::string file_name)
{
	std::string	file_type;

	if (file_name.find(".html") != std::string::npos || file_name.find(".htm") != std::string::npos)
		file_type = "text/html";
	else if (file_name.find(".gif") != std::string::npos)
		file_type = "image/gif";
	else if (file_name.find(".png") != std::string::npos)
		file_type = "image/png";
	else if (file_name.find(".jpg") != std::string::npos)
		file_type = "image/jpeg";
	else if (file_name.find(".mpg") != std::string::npos)
		file_type = "video/mpg";
	else if (file_name.find(".mp4") != std::string::npos)
		file_type = "video/mp4";
	else
		file_type = "text/plain";
	return (file_type);
}

std::string	itos(int num)
{
	std::stringstream	ss;
	ss << num;
	return (ss.str());
}

std::vector<unsigned char>	stou(std::stringstream& ss)
{
	std::string					tmp_string = ss.str();
	std::vector<unsigned char>	result(tmp_string.begin(), tmp_string.end());
	return result;
}

std::vector<unsigned char>	BuildHeader(int status_code, std::map<std::string, std::string>& headers, bool include_crlf)
{
	std::stringstream			ss;

	ss << SERVER_HTTP_VERSION << SPACE << status_code << SPACE << get_status_codes().find(status_code)->second << CRLF;
	ss << "Server:" << SPACE << SERVER_NAME << CRLF;
	for (std::map<std::string, std::string>::iterator header = headers.begin(); header != headers.end(); ++header)
	{
		ss << header->first << COLON << SPACE << header->second << CRLF;
	}
	if (include_crlf)
		ss << CRLF;
	return stou(ss);
}

std::vector<unsigned char>	BuildResponse(int status_code, std::map<std::string, std::string>& headers, std::vector<unsigned char>& body, bool is_static)
{
	std::vector<unsigned char>	response;
	std::cout << BOLDGREEN << "CODE : " << status_code << RESET << "\n";
	
	std::cout << "is_static: " << is_static << std::endl;
	if (is_static)
	{
		headers["Content-Length"] = itos(body.size());
		response = BuildHeader(status_code, headers, true);
	}
	else
	{
		char const * const CRLFCRLF = "\r\n\r\n";
		int pos = std::search(body.begin(), body.end(), &CRLFCRLF[0], &CRLFCRLF[4]) - body.begin();
		headers["Content-Length"] = itos(body.size() - pos - 4);
		std::cout << "BUILD HEADER\n";
		response = BuildHeader(status_code, headers, false);
	}
	response.insert(response.end(), body.begin(), body.end());

	std::cout << "\n  -- <RESPONSE> -- \n";
	return response;
}

void	SetResponse(Client& client, int status_code, std::map<std::string, std::string>& headers, std::vector<unsigned char>& body)
{
	client.response.status_code = status_code;
	client.response.headers = headers;
	client.response.body = body;
}

bool	IsDirectory(std::string path)
{
	struct stat	buf;

	if (stat(path.c_str(), &buf) == 0)
	{
		if (S_ISDIR(buf.st_mode))
			return true;
	}
	return false;
}

bool	IsRegularFile(std::string path)
{	
	struct stat	buf;

	if (stat(path.c_str(), &buf) == 0)
	{
		if (S_ISREG(buf.st_mode))
			return true;
	}
	return false;
}

bool	IsFileReadable(std::string path)
{	
	struct stat	buf;

	if (stat(path.c_str(), &buf) == 0)
	{
		if (S_IRUSR & buf.st_mode)
			return true;
	}
	return false;
}

bool	IsFileExist(std::string path)
{	
	struct stat	buf;

	if (stat(path.c_str(), &buf) == 0)
		return true;
	return false;
}

void	ReadStaticFile(Client& client, std::string& file_name)
{
	struct stat file_stat;
	std::vector<unsigned char> empty_body;

	client.request.file_fd = open(file_name.c_str(), O_RDONLY, 0644);
	fcntl(client.request.file_fd, F_SETFL, O_NONBLOCK, FD_CLOEXEC);
	fstat(client.request.file_fd, &file_stat);
	client.request.file_size = file_stat.st_size;
	if (client.request.file_size == 0)
	{
		SetResponse(client, client.response.status_code, client.request.headers, empty_body);
		client.sendBuffer = BuildResponse(200, client.response.headers, client.response.body);
		client.events->changeEvents(client.getClientFd(), EVFILT_WRITE, EV_ENABLE, 0, 0, &client);
		close(client.request.file_fd);
		client.request.clear();
		client.response.clear();
	}
	else
		client.events->changeEvents(client.request.file_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, &client);
}

void	ServeStatic(Client& client, std::string& path)
{
	std::vector<unsigned char>			body;
	std::map<std::string, std::string>	headers;

	if (!IsFileExist(path))
		return HandleError(client, 404);
	if (!IsRegularFile(path) || !IsFileReadable(path))
		return HandleError(client, 403);

	if (client.request.method.empty())
		return HandleError(client, 404);
	if (client.request.method != "HEAD")
		ReadStaticFile(client, path);
	headers["Connection"] = "keep-alive";
	headers["Content-Type"] = GetFileType(path);
	client.response.status_code = 200;
}
