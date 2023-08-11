#include "DynamicHandler.hpp"

extern char **environ;

std::vector<unsigned char>	DynamicHandler::handle(HttpRequest& request) const
{
	std::vector<unsigned char> result;
    std::string file_type = getFileTypeD(request.file_name);
    std::ostringstream  header;
    std::vector<char> buffer;
    int pipe_fd[2], pipe_fd_back[2];
    char    *empty_list[] = {NULL};

    std::string body(request.body.begin(), request.body.end());
	// std::cout << BOLDCYAN << request.method << RESET << '\n'; // request에 method 변수 없음
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

		if (execve("./cgi-bin/post_echo", empty_list, environ) == -1)
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
		write(pipe_fd[1], body.c_str(), body.size()); // Write body to pipe
		close(pipe_fd[1]);

		char read_buffer[1024];
		ssize_t bytes_read;
		while ((bytes_read = read(pipe_fd_back[0], read_buffer, sizeof(read_buffer))) > 0) {
			buffer.insert(buffer.end(), read_buffer, read_buffer + bytes_read);
		}
		close(pipe_fd[0]);
		wait(NULL);
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

std::string	getFileTypeD(std::string file_name) // makefile 오류나서 dynamic 전용으로 하나 추가함
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
