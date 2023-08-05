#include "ParseRequest.hpp"
#include "Color.hpp"

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

void ParseRequest::parseRequest(Client &c, ResponseMessage &r)
{
	std::string uri = c.getReq().requestTarget;

	if (uri.find("cgi-bin") == std::string::npos)
	{
		processStatic(c, r, uri);
	}
	else
	{
		size_t lc = uri.find('?');
		if (lc != std::string::npos)
			c.getReq().cgi_args = uri.substr(lc + 1);
		c.getReq().fileName = "." + uri.substr(0, lc);
		// processDynamic(c, r);
	}
}

// --- RIGHT HERE PROCESS STATIC --- //

void	processStatic(Client &c, ResponseMessage &r, std::string uri)
{
	struct stat stat_buf;
	int	ret_stat;

	c.getReq().fileName = "./html" + uri;
	ret_stat = stat(c.getReq().fileName.c_str(), &stat_buf);
	if (S_ISDIR(stat_buf.st_mode))
		processDirectory(c);
	else
		std::cout << BOLDMAGENTA << c.getReq().fileName << " is NOT DIRECTORY" << RESET << std::endl;
	MakeStaticResponse(c, r);
}

void	processDirectory(Client &c)
{
	std::cout << BOLDYELLOW << c.getReq().fileName << " is DIRECTORY" << RESET << std::endl;
	DIR	*dir = opendir(c.getReq().fileName.c_str());
	if (dir == NULL)
	{
		std::cout << "404 File not found\n";
		return ;
	}
	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL)
	{
		std::string	fileName = entry->d_name;
		// entry 구조체 dname에는 html 디렉토리의 모든 파일이 저장되어있음 (편리)
		// 모든 파일 네임 확인하려면 아래 주석 풀면 됨
		// std::cout << "Entry filename : " << fileName << '\n';
		if (fileName == "index.html" || fileName == "index.htm")
		{
			struct stat stat_index;
			std::string path = c.getReq().fileName + "index.html";
			int indexStat = stat(path.c_str(), &stat_index);
			if ((indexStat == 0) && S_ISREG(stat_index.st_mode) && (S_IRUSR & stat_index.st_mode))
			{
				c.getReq().fileName = path;
				std::cout << BOLDRED << "PATH : " << path <<RESET << '\n';
			}
		}
	}
	closedir(dir);
}

void	MakeStaticResponse(Client &c, ResponseMessage &r)
{
	c.getReq().fileType = getFileType(c.getReq().fileName);
	std::vector<char>	buffer;
	std::string			header;
	std::ifstream		file(c.getReq().fileName, std::ios::binary);

	file.seekg(0, file.end);
	int length = file.tellg();
	if (length <= 0) // when cannot read file
	{
		return ;
	}
	file.seekg(0, file.beg);
	buffer.resize(length);
	file.read(&buffer[0], length);
	header = build_header("200 OK", length, c.getReq().fileType);
	r.fillResponse(header, buffer);
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

// -------------------------- // 

// --- RIGHT HERE PROCESS DYNAMIC --- //
void	processDynamic(Client &c, ResponseMessage &r, std::string uri)
{
	c.getReq().fileType = getFileType(c.getReq().fileName);
	std::vector<char>	buffer;
	std::string			header;
	int	pipe_fd[2], pipe_fd_back[2];
	char	*empty_list[] = { NULL };
}

ParseRequest::ParseRequest() {}

ParseRequest::~ParseRequest() {}