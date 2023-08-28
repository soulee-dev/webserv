#include "DynamicHandler.hpp"
#include "../../Client.hpp"
#include <fcntl.h>

extern char **environ;
static std::string intToString(int number)
{
	std::stringstream sstream;
	sstream << number;
	return sstream.str();
}


void	OpenFd(Client &client)
{
	Request &request = client.request;

	if (pipe(request.pipe_fd) == -1 || pipe(request.pipe_fd_back) == -1)
	{
		std::cerr << "Pipe error" << std::endl;
		exit(0);
	}
	fcntl(request.pipe_fd[0], F_SETFL, O_NONBLOCK, FD_CLOEXEC);
	fcntl(request.pipe_fd_back[1], F_SETFL, O_NONBLOCK, FD_CLOEXEC);
	fcntl(request.pipe_fd[1], F_SETFL, O_NONBLOCK, FD_CLOEXEC);
	fcntl(request.pipe_fd_back[0], F_SETFL, O_NONBLOCK, FD_CLOEXEC);
}

void	RunCgi(Client& client)
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
		std::string size_str = intToString(size);
		const char *size_cstr = size_str.c_str();

		setenv("QUERY_STRING", request.cgi_args.c_str(), 1);
		setenv("REQUEST_METHOD", request.method.c_str(), 1);
		setenv("CONTENT_LENGTH", size_cstr, 1);
		setenv("SERVER_PROTOCOL", SERVER_HTTP_VERSION, 1);
		setenv("PATH_INFO", request.cgi_path_info.c_str(), 1);
		setenv("HTTP_X_SECRET_HEADER_FOR_TEST", request.headers["x-secret-header-for-test"].c_str(), 1);
		setenv("CONTENT_TYPE", request.headers["content-type"].c_str(), 1);
		setenv("HTTP_COOKIE", request.headers["cookie"].c_str(), 1);

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
		request.pipe_fd[0] = -1;
		close(request.pipe_fd_back[1]);
		request.pipe_fd_back[1] = -1;
	}
}
