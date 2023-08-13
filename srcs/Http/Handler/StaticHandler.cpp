#include "StaticHandler.hpp"
#include "ErrorHandler.hpp"
#include "../../Client.hpp"
#include "ErrorHandler.hpp"

std::vector<unsigned char>	StaticHandler::handle(Client& client) const
{
	HttpRequest&	request = client.httpRequestManager.getRequest();
	std::cout << request.file_name.c_str() << '\n';
	std::vector<unsigned char>	result;
	request.check = 0;

    if (IsDirectory(request.file_name) || (is_directory(request.file_name)))
        return ProcessDirectory(client);
	else // warning 뜸 : Different  indentation for if  and corresponding else
		std::cout << BOLDMAGENTA << "Is NOT DIRECTORY ---> " << request.file_name << RESET << std::endl;
	
	if (!IsRegularFile(request.file_name) || !IsFileReadble(request.file_name)) // chmod 등으로 권한이 없어진 파일
		return ErrorHandler::handler(403);
	else
	{
		// 정상적인 파일
		std::vector<unsigned char>			body;
		std::map<std::string, std::string>	headers;

		body = ReadStaticFile(request.file_name);
		headers["Connection"] = "close";
		headers["Content-Type"] = GetFileType(request.file_name);
		return BuildResponse(200, headers, body);
	}
}

int	is_directory(std::string fileName)
{
	if (fileName.back() == '/')
		return 1;
	return (0);
}

std::vector<unsigned char>	StaticHandler::HandleDirectoryListing(std::string& path) const
{	
	std::vector<unsigned char>			body;
	std::map<std::string, std::string>	headers;

	DIR	*dir = opendir(path.c_str());
	if (dir == NULL) 
	// Error Handler를 호출해야 하는 첫 번째 경우 (errnum = 1), 
	// 현재 default.conf의 root는 /html로 지정되어 있는데, 그 /html이 없는 경우이다.
	{
		return ErrorHandler::handler(404);
	}

	std::stringstream	ss;
	ss << "<html><body><h1>Directory Listing</h1><ul>";
	struct dirent* entry;

    while ((entry = readdir(dir)) != NULL) {
        ss << "<li><a href='" << std::string(entry->d_name) << "'>" << std::string(entry->d_name) + "</a></li>";
    }

    ss << "</ul></body></html>";
	std::cout << BOLDGREEN << "CONTENT HTML : " << ss.str() << '\n';
	closedir(dir);

	body = stou(ss);
	headers["Connection"] = "close";
	headers["Content-Type"] = "text/html";
	return BuildResponse(200, headers, body);
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

std::vector<unsigned char>	StaticHandler::ProcessDirectory(Client& client) const
{
	HttpRequest&	request = client.httpRequestManager.getRequest();

	std::cout << BOLDYELLOW << "Is DIRECTORY ---> " << request.file_name  << RESET << std::endl;
	DIR	*dir = opendir(request.file_name.c_str());
	if (dir == NULL)
		return ErrorHandler::handler(404);
	// Error Handler를 호출해야 하는 첫 번째 경우 (errnum = 1), 
	// 현재 default.conf의 root는 /html로 지정되어 있는데, 그 /html이 없는 경우이다.
	struct dirent* entry;
	std::vector<std::string> fileList;
	while ((entry = readdir(dir)) != NULL)
	{
		fileList.push_back(entry->d_name);
		// 디렉토리 내 파일 이름 확인
	}
	if (compareStaticFile(fileList, request))
	{
		std::string path = request.file_name + "/" + request.target;
		std::cout << " => " << path <<'\n'; 
		if (IsRegularFile(path) && IsFileReadble(path))
		{
			request.check = 1;
			request.file_name = path;
			std::cout << BOLDRED << "PATH : " << path << RESET << '\n';
		}
		else
		{
			// Error Handler를 호출해야 하는 두 번째 경우 (errnum = 2), 
			// index vector내의 파일이 정규 파일이 아닌 경우(ISREG), 
			// 혹은 사용자에게 권한이 주어지지 않은 경우(ISUSR) 
			std::cout << BOLDRED << "Call Error Handler 2\n" << RESET;
			return ErrorHandler::handler(403);
		}
	}
	else
	{
		if (request.isAutoIndex)
		{
			closedir(dir);
			return HandleDirectoryListing(request.file_name);
		}
		else
		{
			// Error Handler를 호출해야 하는 세 번째 경우 (errnum = 3), 
			// root 디렉토리 내에 conf 파일 index에 지정한 파일이 없으면서
			// 동시에 auto index도 아닌 경우
			std::cout << BOLDRED << "Call Error Handler 3\n" << RESET;
			return ErrorHandler::handler(404);
		}
	}
	closedir(dir);
}

StaticHandler::~StaticHandler()
{}

void StaticHandler::sendReqtoEvent(Client &client)
{
	client.sendBuffer = handle(client);
	client.events->changeEvents(client.getClientFd(), EVFILT_WRITE, EV_ENABLE | EV_ADD, 0, 0, &client);
}
