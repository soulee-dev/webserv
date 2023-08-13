#include "DynamicHandler.hpp"
#include "../../Client.hpp"

extern char **environ;

void DynamicHandler::OpenFd(Client &client)
{
	HttpRequest &currRequest = client.httpRequestManager.getRequest();

	if (pipe(currRequest.pipe_fd) == -1 || pipe(currRequest.pipe_fd_back) == -1)
	{
		std::cerr << "Pipe error" << std::endl;
		exit(0);
	}
}

void DynamicHandler::SendReqtoCgi(Client &client)
{
	HttpRequest &request = client.httpRequestManager.getRequest();

    std::string body(request.body.begin(), request.body.end());
	std::cout << BOLDGREEN << "BODY\n" << body << RESET << '\n';
    client.events->changeEvents(request.pipe_fd[1], EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, &client);
}

void DynamicHandler::RunCgi(Client& client)
{
	HttpRequest &request = client.httpRequestManager.getRequest();

	pid_t	pid = fork();
	if (pid == -1)
	{
		std::cerr << "Fork error" << std::endl;
		exit(0);
	}
	if (pid == 0) // 자식 코드
	{
		// Child process
		dup2(request.pipe_fd[0], STDIN_FILENO); // stdin을 pipe_fd[0]로 복제
		dup2(request.pipe_fd_back[1], STDOUT_FILENO); // stdout을 pipe_fd_back[1]로 복제

		close(request.pipe_fd[0]); // Close unused read end
		close(request.pipe_fd_back[1]); // Close unused write end in parent
		close(request.pipe_fd[1]); // Close unused read end
		close(request.pipe_fd_back[0]); // Close unused write end in parent

		int size = request.body.size();
		std::string size_str = std::to_string(size); //c++ 11 
		const char *size_cstr = size_str.c_str();

		std::cout << request.cgi_args << std::endl;
		setenv("QUERY_STRING", request.cgi_args.c_str(), 1);
		setenv("REQUEST_METHOD", request.method.c_str(), 1);
		setenv("CONTENT_LENGTH", size_cstr, 1);

		if (execve(request.path.c_str(), NULL, environ) == -1)
		{
			// TODO: Handle Error
			std::cerr << "execve error" << std::endl;
			exit(0);
		}
	}
	else
	{
		close(request.pipe_fd[0]); // Close unused read end
		close(request.pipe_fd_back[1]); // Close unused write end in parent
	}
}

void DynamicHandler::MakeResponse(Client& client)
{
	client.createResponse();
	ResponseMessage& currRes = client.getBackRes();
	currRes.status_code = 200;
}

void DynamicHandler::ReadFromCgi(Client& client)
{
	HttpRequest &currRequest = client.httpRequestManager.getRequest();
    client.events->changeEvents(currRequest.pipe_fd_back[0], EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, &client);
}

std::vector<unsigned char>	DynamicHandler::handle(Client& client) const
{
	static_cast<void>(client);
	std::vector<unsigned char> result;
	return result;
}

DynamicHandler::~DynamicHandler()
{}
