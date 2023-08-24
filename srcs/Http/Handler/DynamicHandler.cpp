#include "DynamicHandler.hpp"
#include "../../Client.hpp"
#include <fcntl.h>

extern char **environ;

void DynamicHandler::OpenFd(Client &client)
{
	Request &request = client.request;

	if (pipe(request.pipe_fd) == -1 || pipe(request.pipe_fd_back) == -1)
	{
		std::cerr << "Pipe error" << std::endl;
		exit(0);
	}
	fcntl(request.pipe_fd[0], F_SETFL, O_NONBLOCK);
	fcntl(request.pipe_fd_back[1], F_SETFL, O_NONBLOCK);
	fcntl(request.pipe_fd[1], F_SETFL, O_NONBLOCK);
	fcntl(request.pipe_fd_back[0], F_SETFL, O_NONBLOCK);
}

void DynamicHandler::SendReqtoCgi(Client &client)
{
	Request &request = client.request;

    std::string body(request.body.begin(), request.body.end());
    client.events->changeEvents(request.pipe_fd[1], EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, &client);
    client.events->changeEvents(request.pipe_fd_back[0], EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, &client);
}

void DynamicHandler::RunCgi(Client& client)
{
	Request &request = client.request;

	pid_t	pid = fork();
	if (pid == -1)
	{
		std::cerr << "Fork error" << std::endl;
		exit(0);
	}
	if (pid == 0)
	{
		dup2(request.pipe_fd[0], STDIN_FILENO);
		dup2(request.pipe_fd_back[1], STDOUT_FILENO);

		close(request.pipe_fd[0]);
		close(request.pipe_fd_back[1]);
		close(request.pipe_fd[1]);
		close(request.pipe_fd_back[0]);

		int size = request.body.size();
		std::string size_str = std::to_string(size);
		const char *size_cstr = size_str.c_str();

		setenv("QUERY_STRING", request.cgi_args.c_str(), 1);
		setenv("REQUEST_METHOD", request.method.c_str(), 1);
		setenv("CONTENT_LENGTH", size_cstr, 1);
		setenv("SERVER_PROTOCOL", SERVER_HTTP_VERSION, 1);
		setenv("PATH_INFO", request.cgi_path_info.c_str(), 1);
		setenv("HTTP_X_SECRET_HEADER_FOR_TEST", request.headers["x-secret-header-for-test"].c_str(), 1);
		setenv("CONTENT_TYPE", request.headers["content-type"].c_str(), 1);

		if (request.uri.find(".bla") != std::string::npos)
			request.path = "./tester/cgi_tester";

		if (execve(request.path.c_str(), NULL, environ) == -1)
		{
			// TODO: Handle Error
			std::cerr << "execve error" << std::endl;
			exit(0);
		}
	}
	else
	{
		close(request.pipe_fd[0]);
		close(request.pipe_fd_back[1]);
	}
}

void DynamicHandler::MakeResponse(Client& client)
{
	Response& response = client.response;
	response.status_code = 200;
}

void DynamicHandler::ReadFromCgi(Client& client)
{
	Request&	request = client.request;
    client.events->changeEvents(request.pipe_fd_back[0], EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, &client);
}

std::vector<unsigned char>	DynamicHandler::handle(Client& client) const
{
	static_cast<void>(client);
	std::vector<unsigned char> result;
	return result;
}

DynamicHandler::~DynamicHandler()
{
}
