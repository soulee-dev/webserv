#include "StaticHandler.hpp"

std::vector<unsigned char>	StaticHandler::handle(HttpRequest& request) const
{
    struct stat stat_buf;
    int ret_stat = stat(request.file_name.c_str(), &stat_buf);
    if (S_ISDIR(stat_buf.st_mode))
        processDirectory(request);
	else
		std::cout << BOLDMAGENTA << request.file_name << " is NOT DIRECTORY" << RESET << std::endl;
	MakeStaticResponse(request);

	std::vector<unsigned char>	result;
    result.insert(result.end(), request.header.begin(), request.header.end());
    result.insert(result.end(), request.ubuffer.begin(), request.ubuffer.end());
	return (result);
}

void	processDirectory(HttpRequest& request)
{
	std::cout << BOLDYELLOW << request.file_name << " is DIRECTORY" << RESET << std::endl;
	DIR	*dir = opendir(request.file_name.c_str());
	if (dir == NULL)
	{
		std::cout << "404 File not found\n";
		// exit(0);
        return ;
	}
	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL)
	{
		std::string	file_name = entry->d_name;
		// entry 구조체 dname에는 html 디렉토리의 모든 파일이 저장되어있음, 모든 파일 네임 확인하려면 아래 주석 풀면 됨
		// std::cout << "Entry file_name : " << file_name << '\n';
		if (file_name == "index.html" || file_name == "index.htm")
		{
			struct stat stat_index;
			std::string path = request.file_name + "index.html";
			int indexStat = stat(path.c_str(), &stat_index);
			if ((indexStat == 0) && S_ISREG(stat_index.st_mode) && (S_IRUSR & stat_index.st_mode))
			{
				request.file_name = path;
				std::cout << BOLDRED << "PATH : " << path <<RESET << '\n';
			}
		}
	}
	closedir(dir);
}

void	MakeStaticResponse(HttpRequest& request)
{
	std::string file_type = getFileType(request.file_name);
	std::vector<char>	buffer;
	std::ifstream		file(request.file_name, std::ios::binary);

	file.seekg(0, file.end);
	int length = file.tellg();
	if (length <= 0) // when cannot read file
	{
		return ;
	}
	file.seekg(0, file.beg);
	buffer.resize(length);
	file.read(&buffer[0], length);
	request.header = build_header("200 OK", length, file_type);
    request.ubuffer.insert(request.ubuffer.end(), buffer.begin(), buffer.end());
}

std::string	build_header(std::string status_code, int file_size, std::string file_type)
{
	std::ostringstream	header;

	header << "HTTP/1.1 " << status_code << CRLF;
	header << "Server: " << "Master J&J Server" << CRLF;
	header << "Connection: close" << CRLF;
	header << "Content-length: " << file_size << CRLF;
	header << "Content-type: " << file_type << CRLF;
	header << CRLF;

	return (header.str());
}

std::string	getFileType(std::string file_name)
{
	std::string	file_type;

	if (file_name.find(".html") != std::string::npos || file_name.find(".htm") != std::string::npos)
		file_type = "text/html";
	else if (file_name.find(".gif") != std::string::npos)
		file_type = "image/gif";
	else if (file_name.find(".png") != std::string::npos || file_name.find(".ico") != std::string::npos)
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
