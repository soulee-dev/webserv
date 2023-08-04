#include "ServerManager.hpp"
#include "ResponseMessageWriter.hpp"
#include "Server.hpp"
#include <cstdlib>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include "utils.hpp"
#include "Color.hpp"

void ServerManager::setServers(std::map<PORT, std::vector<Server> >& servers)
{
	this->servers = servers;
}

void ServerManager::exitWebServer(std::string str)
{
	std::cout << str << std::endl;
	exit(1);
}

ServerManager& ServerManager::getInstance()
{
	static ServerManager instance;
	return instance;
}

ServerManager::ServerManager()
	: LISTENCAPACITY(5)
{
	messageReader = &RequestMessageReader::getInstance();
	messageWriter = &ResponseMessageWriter::getInstance();
}

ServerManager::~ServerManager()
{
	close(events.getKq());
}

void ServerManager::disconnectServer(int serverFd)
{
	std::cout << "Server disconnected: " << serverFd << std::endl;
	events.changeEvents(serverFd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
	close(serverFd);
}

void ServerManager::disconnectClient(int clientFd)
{
	messageReader->deleteClient(clientFd);
	messageWriter->deleteClient(clientFd);
	std::cout << "Server disconnected: " << clientFd << std::endl;
	close(clientFd);
}

Server* ServerManager::getClientServer(SOCKET ident)
{
	Server* res = NULL;

	Client& currClient = clientsBySocket[ident];
	PORT currPort = currClient.getPort();
	serverName clientHost = currClient.getReq()->headers["host"];

	std::vector<Server>::iterator iter = servers[currPort].begin();

	if (iter == servers[currPort].end())
		return res;
	res = &servers[currPort][0];
	while (iter != servers[currPort].end())
	{
		if (iter->getServerName().compare(clientHost) == 0)
		{
			res = &(*iter);
			break;
		}
		iter++;
	}
	return res;
}

void ServerManager::insertClient(SOCKET ident)
{
	clientsBySocket.insert(std::pair<SOCKET, Client>(ident, Client()));
}

static std::string intToString(int n)
{
	std::string res;
	std::stringstream sstream;

	sstream << n;
	sstream >> res;
	return res;
}

int ServerManager::openPort(ServerManager::PORT port, Server& firstServer)
{
	struct addrinfo* info;
	struct addrinfo hint;
	struct sockaddr_in socketaddr;
	int opt = 1;

	memset(&hint, 0, sizeof(struct addrinfo));
	memset(&socketaddr, 0, sizeof(struct sockaddr_in));
	socketaddr.sin_family = AF_INET;
	std::cout << "Port number : " << port << std::endl;

	socketaddr.sin_port = htons(port);
	socketaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	hint.ai_family = AF_INET;
	hint.ai_socktype = SOCK_STREAM;

	std::string strPortNumber = intToString(port);
	int errorCode = getaddrinfo(firstServer.getServerName().c_str(), strPortNumber.c_str(), &hint, &info);
	if (errorCode == -1)
		exitWebServer(gai_strerror(errorCode));

	SOCKET serverSocket = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
	if (serverSocket == -1)
		exitWebServer("socket() error");
	setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	errorCode = bind(serverSocket, reinterpret_cast<struct sockaddr*>(&socketaddr), sizeof(socketaddr));
	if (errorCode)
		exitWebServer("bind() error");
	errorCode = listen(serverSocket, LISTENCAPACITY);
	if (errorCode)
		exitWebServer("listen() error");
	return serverSocket;
}

void ServerManager::initServers(void)
{
	std::map<PORT, std::vector<Server> >::iterator portIter = servers.begin();

	if (events.initKqueue())
	{
		std::cout << "kqueue() error" << std::endl;
		exit(1);
	}
	while (portIter != servers.end())
	{
		// std::vector<Server>::iterator serverIter = portIter->second.begin();
		Server& firstServer = portIter->second.front();
		SOCKET serversSocket = openPort(portIter->first, firstServer);
		fcntl(serversSocket, F_SETFL, O_NONBLOCK);

		portByServerSocket[serversSocket] = portIter->first;
		events.changeEvents(serversSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
		// udata 에 portIter->second 넣어도 될듯?
		portIter++;
	}
}

void ServerManager::errorEventProcess(SOCKET ident)
{
	if (portByServerSocket.find(ident) != portByServerSocket.end())
	{
		std::cout << "Server socket error" << std::endl;
		disconnectServer(ident);
	}
	else
	{
		std::cout << "Client socket error" << std::endl;
		disconnectClient(ident);
	}
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

void	MakeStaticResponse(RequestMessage& req, std::vector<char>& response)
{
	// std::cout << BOLDGREEN << "filename : " << req.fileName << RESET << '\n';
	req.fileType = getFileType(req.fileName);
	// std::string	file_type = "text/html";
	std::vector<char>   buffer;
	std::string     header;
	std::ifstream   file(req.fileName, std::ios::binary);

	file.seekg(0, file.end);
	int length = file.tellg();
	if (length <= 0) {
		// Error when can't read file
		return;
	}
	file.seekg(0, file.beg);
	buffer.resize(length);
	file.read(&buffer[0], length);
	header = utils::build_header("200 OK", length, req.fileType);
	response.insert(response.end(), header.begin(), header.end());
	response.insert(response.end(), buffer.begin(), buffer.end());
	std::cout << BOLDGREEN << req.fileName << "'s FILE TYPE : " << req.fileType << RESET << std::endl;
}


extern char **environ;

void	MakeDynamicResponse(RequestMessage& request, std::vector<char>& response)
{
	std::vector<char> buffer;
	std::string	file_type = "text/html";
	std::ostringstream	header;
	int	pipe_fd[2], pipe_fd_back[2];
	char	*empty_list[] = {NULL};

	std::string body(request.body.begin(), request.body.end());
	std::cout << BOLDCYAN << request.method << RESET << '\n';
	std::cout << BOLDGREEN << "BODY : " << body << RESET << '\n';

	if (pipe(pipe_fd) == -1 || pipe(pipe_fd_back) == -1)
	{
		std::cerr << "Pipe error" << std::endl;
		return ;
	}

	pid_t	pid = fork();
	if (pid == -1)
	{
		std::cerr << "Fork error" << std::endl;
		return ;
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
			return ;
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
	static_cast<void>(request);
	header << "HTTP/1.1 200 OK" << CRLF;
	header << "Server: " << SERVER_NAME << CRLF;
	std::string	str_header = header.str();
	response.insert(response.end(), str_header.begin(), str_header.end());
	response.insert(response.end(), buffer.begin(), buffer.end());
}

int	ParseURI(std::string uri, RequestMessage &req)
{
	if (req.requestTarget.find("cgi-bin") == std::string::npos)
	{
		uri = req.requestTarget;
		req.fileName = "./html" + uri;
		// if (uri[uri.length() - 1] == '/') // might be unused
		// 	req.is_directory = true;
		return 0; // this means file is STATIC;
	}
	else
	{
		size_t lc = uri.find('?');
		if (lc != std::string::npos)
			req.cgi_args = uri.substr(lc + 1);
		req.fileName = "." + uri.substr(0, lc);
		return 1; // this means file is DYNAMIC;
	}
}

void ServerManager::readEventProcess(SOCKET ident)
{
	if (isRespondToServer(ident))
		acceptClient(ident);
	else
	{
		events.changeEvents(ident, EVFILT_TIMER, EV_EOF, NOTE_SECONDS, 1000, NULL);
		if (messageReader->readMessage(ident))
		{
			disconnectClient(ident);
		}
		else if (messageReader->ParseState[ident] == DONE || messageReader->ParseState[ident] == ERROR)
		{
			// 메시지 처리하여 버퍼에 입력해야함.
			// events.changeEvents(ident, EVFILT_WRITE, EV_ENABLE, 0, 0, NULL);
			struct stat	stat_buf;
			int	ret_stat;
			std::cout << "메시지 잘 받았습니다^^" << std::endl;
			RequestMessage  request = messageReader->getInstance().messageBuffer[ident];
			std::vector<char>   response;
			std::cout << BOLDCYAN << "METHOD : " << request.method << RESET << '\n';
			if (!ParseURI(request.requestTarget, request))
			{
				std::cout << BOLDMAGENTA << "REQUEST FILENAME : " << request.fileName << RESET << '\n';
				ret_stat = stat(request.fileName.c_str(), &stat_buf);
				if (S_ISDIR(stat_buf.st_mode))
				{
					std::cout << BOLDYELLOW << request.fileName << " is DIRECTORY" << RESET << std::endl;
					DIR	*dir = opendir(request.fileName.c_str());
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
							std::string path = request.fileName + "index.html";
							int indexStat = stat(path.c_str(), &stat_index);
							if ((indexStat == 0) && S_ISREG(stat_index.st_mode) && (S_IRUSR & stat_index.st_mode))
							{
								request.fileName = path;
								std::cout << BOLDRED << "PATH : " << path <<RESET << '\n';
								MakeStaticResponse(request, response);
							}
						}
					}
					closedir(dir);
				}
				else
				{
					std::cout << BOLDBLUE << request.fileName << " is NOT directory" << RESET << std::endl;
					MakeStaticResponse(request, response);
				}
			}
			else
			{
				MakeDynamicResponse(request, response);
			}
			if (request.fileType.find("text") != std::string::npos)
			{
				for (std::vector<char>::iterator it = response.begin(); it != response.end(); ++it)
				{
					std::cout << *it;
				}
			}
			else
			{
				std::cout << "----------------------------------------\n";
				std::cout << "      file type이 " << BOLDGREEN << request.fileType << RESET << " 이에요\n";
				std::cout << "헤더는 일단 text 파일만 출력하도록 했어요\n";
				std::cout << "----------------------------------------";
			}
			std::cout << std::endl;
			messageWriter->writeBuffer[ident].insert(messageWriter->writeBuffer[ident].end(), response.begin(), response.end());
			events.changeEvents(ident, EVFILT_WRITE, EV_ENABLE, 0, 0, NULL);
			messageReader->ParseState[ident] = METHOD;
			messageReader->messageBuffer[ident].clear();
		}
	}
}

void ServerManager::writeEventProcess(SOCKET ident)
{
	size_t n;
	n = write(ident, &*messageWriter->writeBuffer[ident].begin(), messageWriter->writeBuffer[ident].size());
	events.changeEvents(ident, EVFILT_WRITE, EV_DISABLE, 0, 0, NULL);
}

void ServerManager::timerEventProcess(SOCKET ident)
{
	events.changeEvents(ident, EVFILT_TIMER, EV_DELETE, 0, 0, NULL);
	disconnectClient(ident);
}

bool ServerManager::isRespondToServer(SOCKET serverSocket)
{
	return portByServerSocket.find(serverSocket) != portByServerSocket.end();
}

void ServerManager::acceptClient(SOCKET serverSocket)
{
	const int clientSocket = accept(serverSocket, NULL, NULL);
	if (clientSocket == -1)
	{
		std::cout << "accept() error" << std::endl;
		return;
	}
	fcntl(clientSocket, F_SETFL, O_NONBLOCK);
	events.changeEvents(clientSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	events.changeEvents(clientSocket, EVFILT_TIMER, EV_ADD | EV_ENABLE, NOTE_SECONDS, 1000, NULL);
	events.changeEvents(clientSocket, EVFILT_WRITE, EV_ADD | EV_DISABLE, 0, 0, NULL);
	messageReader->insertNewClient(clientSocket);
	messageWriter->insertNewClient(clientSocket);
	insertClient(clientSocket);
}

void ServerManager::runServerManager(void)
{
	int newEvent;

	while (1)
	{
		newEvent = events.newEvents();
		if (newEvent == -1)
			exitWebServer("kevent() error");
		events.clearChangeEventList();
		for (int i = 0; i != newEvent; i++)
		{
			struct kevent& currEvent = events[i];

			if (currEvent.flags & EV_ERROR)
			{
				errorEventProcess(currEvent.ident);
				continue;
			}
			switch (currEvent.filter)
			{
			case EVFILT_READ:
				readEventProcess(currEvent.ident);
				break;
			case EVFILT_WRITE:
				writeEventProcess(currEvent.ident);
				break;
			case EVFILT_TIMER:
				timerEventProcess(currEvent.ident);
				break;
			}
		}
	}
}
