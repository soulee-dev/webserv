#include "DynamicHandler.hpp"

extern char	**environ;

std::vector<unsigned char>	DynamicHandler::handle(HttpRequest& request)
{
	std::vector<char>	buffer;
	int					pipe_fd[2];
	int					pipe_fd_back[2];
	char				*empty_list[] = { NULL };

	if (pipe(pipe_fd) == -1 || pipe(pipe_fd_back) == -1)
	{
		// TODO handle error;
		
	}
	pid_t	pid = fork();
	if (pid == -1)
	{
		// TODO handle error
	}
	if (pid == 0)
	{
		close(pipe_fd[1]);
		close(pipe_fd_back[0]);
		dup2(pipe_fd[0], STDIN_FILENO);
		dup2(pipe_fd_back[1], STDOUT_FILENO);

		size_t	size = request.requestMessage.body.size();
		setenv("REQUEST_METHOD", request.requestMessage.method.c_str(), 1);
		setenv("CONTENT_LENGTH", itos(request.requestMessage.body.size()).c_str(), 1);
		setenv("QUERY_STRING", request.cgi_args.c_str(), 1);

		close(pipe_fd[0]);
		if (execve(request.file_name.c_str(), empty_list, environ) == -1)
		{
			// TODO handle error
		}
	}
	else
	{
		close(pipe_fd[0]);
		close(pipe_fd_back[1]);
		write(pipe_fd[1], &request.requestMessage.body[0], request.requestMessage.body.size());
		close(pipe_fd[1]);

		char 	read_buffer[1024];
		ssize_t bytes_read;

		while ((bytes_read = read(pipe_fd_back[0], read_buffer, sizeof(read_buffer))) > 0) {
			buffer.insert(buffer.end(), read_buffer, read_buffer + bytes_read);
		}
		close(pipe_fd[0]);
		wait(NULL);
	}
	buildHeader(200);
	response.insert(response.end(), header.begin(), header.end());
	response.insert(response.end(), buffer.begin(), buffer.end());
	return (response);
}