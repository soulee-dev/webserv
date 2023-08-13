#include "Handler.hpp"

std::string	Handler::getFileType(std::string file_name) const
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

std::string	Handler::itos(int num)
{
	std::stringstream	ss;
	ss << num;
	return (ss.str());
}

std::vector<unsigned char>	Handler::stou(std::stringstream& ss)
{
	std::string					tmp_string = ss.str();
	std::vector<unsigned char>	result(tmp_string.begin(), tmp_string.end());
	return result;
}

// void	Handler::buildHeader(int status_code)
// {
// 	std::stringstream			ss;

// 	// start-line
// 	ss << SERVER_HTTP_VERSION << SPACE << status_code << SPACE << get_status_codes().find(status_code)->second << CRLF;
// 	ss << "Server:" << SPACE << SERVER_NAME << CRLF;
// 	for (std::map<std::string, std::string>::iterator header = headers.begin(); header != headers.end(); ++header)
// 	{
// 		ss << header->first << COLON << SPACE << header->second << CRLF;
// 	}
// 	ss << CRLF;
// 	header = stou(ss);
// }

bool	Handler::IsDirectory(std::string path)
{
	struct stat	buf;

	if (stat(path.c_str(), &buf) == 0)
	{
		if (S_ISDIR(buf.st_mode))
			return true;
	}
	return false;
}

bool	Handler::IsRegularFile(std::string path)
{	
	struct stat	buf;

	if (stat(path.c_str(), &buf) == 0)
	{
		if (S_ISREG(buf.st_mode))
			return true;
	}
	return false;
}

bool	Handler::IsFileReadble(std::string path)
{	
	struct stat	buf;

	if (stat(path.c_str(), &buf) == 0)
	{
		if (S_IRUSR & buf.st_mode)
			return true;
	}
	return false;
}

bool	Handler::IsFileExist(std::string path)
{	
	struct stat	buf;

	if (stat(path.c_str(), &buf) == 0)
		return true;
	return false;
}

Handler::~Handler()
{}