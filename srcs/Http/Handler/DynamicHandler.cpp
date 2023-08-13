#include "DynamicHandler.hpp"
#include "../../Client.hpp"

extern char **environ;

void DynamicHandler::openFd(Client &client)
{
	HttpRequest &currRequest = client.httpRequestManager.getRequest();

	if (pipe(currRequest.pipe_fd) == -1 || pipe(currRequest.pipe_fd_back) == -1)
	{
		std::cerr << "Pipe error" << std::endl;
		exit(0);
	}
}

void DynamicHandler::sendReqtoCgi(Client &client)
{

	HttpRequest &currRequest = client.httpRequestManager.getRequest();

    std::string body(currRequest.body.begin(), currRequest.body.end());
	std::cout << BOLDGREEN << "BODY\n" << body << RESET << '\n';
    client.events->changeEvents(currRequest.pipe_fd[1], EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, &client);
}

void DynamicHandler::runCgi(Client &client)
{
	pid_t	pid = fork();
	HttpRequest &currRequest = client.httpRequestManager.getRequest();
	if (pid == -1)
	{
		std::cerr << "Fork error" << std::endl;
		exit(0);
	}
	if (pid == 0) // 자식 코드
	{

		// Child process
		dup2(currRequest.pipe_fd[0], STDIN_FILENO); // stdin을 pipe_fd[0]로 복제
		dup2(currRequest.pipe_fd_back[1], STDOUT_FILENO); // stdout을 pipe_fd_back[1]로 복제

		close(currRequest.pipe_fd[0]); // Close unused read end
		close(currRequest.pipe_fd_back[1]); // Close unused write end in parent
		close(currRequest.pipe_fd[1]); // Close unused read end
		close(currRequest.pipe_fd_back[0]); // Close unused write end in parent

		int size = currRequest.body.size();
		std::string size_str = std::to_string(size); //c++ 11 
		const char *size_cstr = size_str.c_str();

		setenv("REQUEST_METHOD", "POST", 1);
		setenv("CONTENT_LENGTH", size_cstr, 1);
		

		if (execve("./www/cgi-bin/post_echo", NULL, environ) == -1)
		{
			std::cerr << "execve error" << std::endl;
			exit(0);
		}
	}
	else
	{
		close(currRequest.pipe_fd[0]); // Close unused read end
		close(currRequest.pipe_fd_back[1]); // Close unused write end in parent
		// wait(NULL);
	}
}

void DynamicHandler::makeResponse(Client& client)
{
	client.createResponse();
	ResponseMessage& currRes = client.getBackRes();
	currRes.startLine = "HTTP/1.1 200 OK";
	currRes.headers["Server"] = "soulee king JJang";
}

void DynamicHandler::readFromCgi(Client& client)
{
	HttpRequest &currRequest = client.httpRequestManager.getRequest();
    client.events->changeEvents(currRequest.pipe_fd_back[0], EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, &client);
}


std::vector<unsigned char>	DynamicHandler::handle(Client& client) const
{
	HttpRequest&	request = client.httpRequestManager.getRequest();

	std::vector<unsigned char> result;
    std::string file_type = getFileType(request.file_name);
    std::ostringstream  header;
    std::vector<char> buffer;
    int pipe_fd[2], pipe_fd_back[2];
    char    *empty_list[] = {NULL};

    std::string body(request.body.begin(), request.body.end());
	std::cout << BOLDGREEN << "BODY\n" << body << RESET << '\n';

	if (pipe(pipe_fd) == -1 || pipe(pipe_fd_back) == -1)
	{
		std::cerr << "Pipe error" << std::endl;
		exit(0);
	}

	pid_t	pid = fork();
	if (pid == -1)
	{
		std::cerr << "Fork error" << std::endl;
		exit(0);
	}
	if (pid == 0) // 자식 코드
	{
		// Child process
		close(pipe_fd[1]);  // Close unused write end
		close(pipe_fd_back[0]); // Close unused read end in child
		dup2(pipe_fd[0], STDIN_FILENO); // stdin을 pipe_fd[0]로 복제
		dup2(pipe_fd_back[1], STDOUT_FILENO); // stdout을 pipe_fd_back[1]로 복제

		int size = body.size();
		std::string size_str = std::to_string(size);
		const char *size_cstr = size_str.c_str();

		setenv("REQUEST_METHOD", "POST", 1);
		setenv("CONTENT_LENGTH", size_cstr, 1);
		
		close(pipe_fd[0]);

		if (execve("./www/cgi-bin/post_echo", empty_list, environ) == -1)
		{
			std::cerr << "execve error" << std::endl;
			exit(0);
		}
	}
	else
	{
		// Parent process
		close(pipe_fd[0]); // Close unused read end
		close(pipe_fd_back[1]); // Close unused write end in parent
		client.events->changeEvents(pipe_fd[1], EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, &request);
	}
	header << "HTTP/1.1 200 OK" << CRLF;
	header << "Server: Master J&J" << CRLF;
	std::string	str_header = header.str();
	request.header.insert(request.header.end(), str_header.begin(), str_header.end());
	request.ubuffer.insert(request.ubuffer.end(), buffer.begin(), buffer.end());

	result.insert(result.end(), request.header.begin(), request.header.end());
    result.insert(result.end(), request.ubuffer.begin(), request.ubuffer.end());
	return result;
}

DynamicHandler::~DynamicHandler()
{}
