#include "StaticHandler.hpp"
#include "ErrorHandler.hpp"

std::vector<unsigned char>	StaticHandler::handle(HttpRequest& request) const
{
    struct stat stat_buf;
    int ret_stat = stat(request.file_name.c_str(), &stat_buf);
	std::vector<unsigned char>	result;
	request.check = 0;

    if (S_ISDIR(stat_buf.st_mode) || (is_directory(request.file_name)))
	{
        processDirectory(request);
		if (request.errnum > 0 || (request.check == 0 && request.isAutoIndex == 1))
		{
			result.insert(result.end(), request.header.begin(), request.header.end());
   	 		result.insert(result.end(), request.ubuffer.begin(), request.ubuffer.end());
			return (result);
		}
	}
	else
		std::cout << BOLDMAGENTA << "Is NOT DIRECTORY ---> " << request.file_name << RESET << std::endl;

	if (!(S_IRUSR & stat_buf.st_mode)) // chmod 등으로 권한이 없어진 파일
	{
		std::cout << "S_IRUSR Error\n";
		std::string header = "<h1>403 Forbidden</h1>";
		std::string buffer = "<h1>403 Forbidden</h1>";
		request.header = build_header("403 forbidden", header.length(), "text/html");
	    request.ubuffer.insert(request.ubuffer.end(), buffer.begin(), buffer.end());
	}
	else if (!S_ISREG(stat_buf.st_mode)) // 정규 파일이 아닌 경우(디렉토리, 파이프 등등)
	{
		std::cout << "S_ISREG Error\n";
		std::string header = "<h1>403 Forbidden</h1>";
		std::string buffer = "<h1>403 Forbidden</h1>";
		request.header = build_header("403 forbidden", header.length(), "text/html");
	    request.ubuffer.insert(request.ubuffer.end(), buffer.begin(), buffer.end());
	}
	else
	{
		MakeStaticResponse(request);
	}
	// std::cout << "header: ";
	// for (size_t i = 0; i < request.header.size(); i++)
	// 	std::cout << request.header[i];
    result.insert(result.end(), request.header.begin(), request.header.end());
    result.insert(result.end(), request.ubuffer.begin(), request.ubuffer.end());
	return (result);
}

int	is_directory(std::string fileName)
{
	if (fileName.back() == '/')
		return 1;
	return (0);
}

void	handleDirectoryListing(HttpRequest& request)
{
	DIR	*dir = opendir(request.file_name.c_str());
	if (dir == NULL) 
	// Error Handler를 호출해야 하는 첫 번째 경우 (errnum = 1), 
	// 현재 default.conf의 root는 /html로 지정되어 있는데, 그 /html이 없는 경우이다.
	{
		request.errnum = 1;
		std::cout << "404 File not found\n"; 
		std::cout << BOLDRED << "Call Error Handler 1\n" << RESET;
		ErrorHandler err;
        return ;
	}

	std::string html = "<html><body><h1>Directory Listing</h1><ul>";
	struct dirent* entry;

    while ((entry = readdir(dir)) != NULL) {
        html += "<li><a href='" + std::string(entry->d_name) + "'>" + std::string(entry->d_name) + "</a></li>";
    }

    html += "</ul></body></html>";
	closedir(dir);

	request.header = build_header("200 OK", html.length(), "text/html");
    request.ubuffer.insert(request.ubuffer.end(), html.begin(), html.end());
}

int	compareStaticFile(std::vector<std::string> fileList, HttpRequest& request)
{
	std::vector<std::string> indexList = request.indexList;

	for (int i = 0; i < indexList.size(); i++)
	{
		std::string	target = indexList[i];
		for (int j = 0; j < fileList.size(); j++)
		{
			if (target == fileList[j])
			{
				request.target = target;
				return 1;
			}
		}
	}
	return 0;
}

void	processDirectory(HttpRequest& request)
{
	std::cout << BOLDYELLOW << "Is DIRECTORY ---> " << request.file_name  << RESET << std::endl;
	DIR	*dir = opendir(request.file_name.c_str());
	if (dir == NULL) 
	// Error Handler를 호출해야 하는 첫 번째 경우 (errnum = 1), 
	// 현재 default.conf의 root는 /html로 지정되어 있는데, 그 /html이 없는 경우이다.
	{
		request.errnum = 1;
		std::cout << "404 File not found\n"; 
		std::cout << BOLDRED << "Call Error Handler 1\n" << RESET;
       	ErrorHandler	err;
		err.handle(request);
		return ;
	}
	struct dirent* entry;
	std::vector<std::string> fileList;
	while ((entry = readdir(dir)) != NULL)
	{
		fileList.push_back(entry->d_name);
		// std::cout << BOLDBLACK << "ENTRY : " << fileList.back() << '\n'; 
		// 디렉토리 내 파일 이름 확인
	}
	if (compareStaticFile(fileList, request))
	{
		struct stat stat_index;
		std::string path = request.file_name + "/" + request.target;
		int indexStat = stat(path.c_str(), &stat_index);
		if ((indexStat == 0) && S_ISREG(stat_index.st_mode) && (S_IRUSR & stat_index.st_mode))
		{
			request.check = 1;
			request.file_name = path;
			std::cout << BOLDGREEN << "PATH : " << path << RESET << '\n';
		}
		else
		{
			// Error Handler를 호출해야 하는 두 번째 경우 (errnum = 2), 
			// index vector내의 파일이 정규 파일이 아닌 경우(ISREG), 
			// 혹은 사용자에게 권한이 주어지지 않은 경우(ISUSR) 
			std::cout << BOLDRED << "Call Error Handler 2\n" << RESET;
			request.errnum = 2;
			return ;
		}
	}
	else
	{
		if (request.isAutoIndex)
		{
			request.check = 0;
			closedir(dir);
			handleDirectoryListing(request);
			return ;
		}
		else
		{
			// Error Handler를 호출해야 하는 세 번째 경우 (errnum = 3), 
			// root 디렉토리 내에 conf 파일 index에 지정한 파일이 없으면서
			// 동시에 auto index도 아닌 경우
			std::cout << BOLDRED << "Call Error Handler 3\n" << RESET;
			request.errnum = 3;
       		ErrorHandler	err;
			err.handle(request);
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
	header << "Content-Type: " << file_type << CRLF;
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
