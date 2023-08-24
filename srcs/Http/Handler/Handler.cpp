#include "Handler.hpp"
#include "HttpStatusCodes.hpp"
#include "ErrorHandler.hpp"
#include <algorithm>

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

std::vector<unsigned char>	BuildResponse(int status_code, std::map<std::string, std::string>& headers, std::vector<unsigned char>& body, bool is_cgi)
{
	std::vector<unsigned char>	response;
	std::cout << BOLDGREEN << "CODE : " << status_code << RESET << "\n";
	
	if (is_cgi)
	{
		char const * const CRLFCRLF = "\r\n\r\n";
		int pos = std::search(body.begin(), body.end(), &CRLFCRLF[0], &CRLFCRLF[4]) - body.begin();
		headers["Content-Length"] = itos(body.size() - pos - 4);
		std::cout << "BUILD HEADER\n";
		response = BuildHeader(status_code, headers, false);
	}
	else
	{
		headers["Content-Length"] = itos(body.size());
		response = BuildHeader(status_code, headers, true);
	}
	response.insert(response.end(), body.begin(), body.end());

	std::cout << "\n  -- <RESPONSE> -- \n";
	return response;
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

std::vector<unsigned char>	ReadStaticFile(std::string& file_name)
{
	std::ifstream	file(file_name.c_str(), std::ios::in | std::ios::binary);

	file.seekg(0, std::ios::end);
	int length = file.tellg();
	file.seekg(0, std::ios::beg);

	std::vector<unsigned char> buffer(length);
	file.read(reinterpret_cast<char*>(&buffer[0]), length);
	return buffer;
}

std::vector<unsigned char>	ServeStatic(Client& client, std::string& path, std::string method)
{
	std::vector<unsigned char>			body;
	std::map<std::string, std::string>	headers;

	if (!IsFileExist(path))
		return ErrorHandler::handle(client, 404);
	if (!IsRegularFile(path) || !IsFileReadable(path))
		return ErrorHandler::handle(client, 403);

	if (method != "HEAD")
		body = ReadStaticFile(path);

	if (method.empty())
		return ErrorHandler::handle(client, 404);

	headers["Connection"] = "keep-alive";
	headers["Content-Type"] = GetFileType(path);
	return BuildResponse(200, headers, body);
}
