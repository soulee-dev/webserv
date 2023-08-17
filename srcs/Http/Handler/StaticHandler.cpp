#include "StaticHandler.hpp"
#include "ErrorHandler.hpp"
#include "../../Client.hpp"
#include "ErrorHandler.hpp"

std::vector<unsigned char>	StaticHandler::handle(Client& client) const
{
	HttpRequest&				request = client.httpRequestManager.getRequest();
	std::vector<unsigned char>	result;

	std::cout << "METHOD : " << request.method << RESET << '\n';

	if (request.method == "POST") // CGI-BIN 없는 POST 요청의 경우 일괄적으로 405를 띄워 줍니다.
		return ErrorHandler::handle(client, 405);

	// TODO: 현재 STATIC으로 들어오는 HEAD 요청에 대해 전부 405를 반환하는데 이것은 테스터가 URI "/"로 보내는 요청의
	// allowed method가 GET 뿐이기 때문입니다. 현재 ServeStatic() 끝에 HEAD일 경우 바디 때는 처리가 되어있어서
	// HEAD를 따로 구현할 건 없는데 allowed method 처리 되면 이 분기문은 없애야합니다.
	else if (request.method == "HEAD")
		return ErrorHandler::handle(client, 405);

	else if (request.method == "PUT") // POST와 PUT이 괴상하게 섞인 분기문을 정리했습니다.
	{
		std::ifstream	ifs(request.path);
		std::ofstream	ofs;
		int res = ifs.is_open();
		ifs.close(); // infile close
		ofs.open(request.path, std::ios::out | std::ios::trunc); // 출력 모드로, 이미 파일이 존재한다면 파일을 비우고 새로 엽니다.
		std::map<std::string, std::string>  headers;
		if (ofs.fail()) // 파일 열기에 실패했으면 404 에러를 호출합니다
			ErrorHandler::handle(client, 404);
		headers["Connection"] = "close";
		// request.body.resize(100); // TODO: max body size
		for (size_t i = 0; i < request.body.size(); i++)
			ofs << request.body[i];
		ofs.close();

		std::vector<unsigned char> empty_body; // PUT 메서드에 응답용으로 만든 빈 body입니다. 이대로 보내면 빈 body를 보내게 됨니다.
		if (res) // curl -v -X DELETE -d "body" http://localhost/put_test/file_should_exist_after
			return BuildResponse(200, headers, empty_body); // 파일 지우고 200, 201 테스트해보려면 위에 curl 입력하면 됩니다.
		return BuildResponse(201, headers, empty_body); // 200이면 이미 있는 파일을 연거고, 201이면 새로 만든 겁니다.
	}

    if (IsDirectory(request.path)) // 나머지 GET 요청에 대한 처리입니다.
        return ProcessDirectory(client);
	return ServeStatic(client, request.path, request.method);
}

int	is_directory(std::string fileName)
{
	if (fileName.back() == '/')
		return 1;
	return (0);
}

std::vector<unsigned char>	StaticHandler::HandleDirectoryListing(Client& client, HttpRequest& request) const
{
	std::vector<unsigned char>			body;
	std::map<std::string, std::string>	headers;

	DIR	*dir = opendir(request.path.c_str());
	if (!dir)
	// Error Handler를 호출해야 하는 첫 번째 경우 (errnum = 1),
	// 현재 default.conf의 root는 /html로 지정되어 있는데, 그 /html이 없는 경우이다.
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
    HttpRequest& request = client.httpRequestManager.getRequest();
    std::vector<std::string> indexVec = request.location.getIndex(); // 벡터에 대한 참조
    std::vector<std::string>::iterator it;

    if (!indexVec.empty()) // 벡터가 비어있지 않은지 확인
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
