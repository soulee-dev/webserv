#include "ServerManager.hpp"
#include "RequestMessageReader.hpp"
#include "algorithm"
#include <iostream>
#include <sstream>

ServerManager::ServerManager()
{
    this->kq = kqueue();
    if (this->kq == -1)
    {
        std::cout << "kqueue() error" << std::endl;
        exit(1);
    }
}

ServerManager::~ServerManager()
{
    close(this->kq);
}

void ServerManager::change_events(int socket, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void* udata)
{
    struct kevent event;
    EV_SET(&event, socket, filter, flags, fflags, data, udata);
    this->change_list.push_back(event);
}

void ServerManager::disconnect_server(int server_fd)
{
    std::cout << "server disconnected: " << server_fd << std::endl;
    close(server_fd);
}

void ServerManager::disconnect_client(int client_fd)
{
    std::cout << "client disconnected: " << client_fd << std::endl;
    close(client_fd);
}

int ServerManager::getKq()
{
    return this->kq;
}

void ServerManager::init_server()
{
    std::map<PORT, std::vector<Server> >::iterator it;
    struct addrinfo* info;
    struct addrinfo hint;
    struct sockaddr_in socketaddr;
    int server_socket;
    int errorcode;
    std::string strPortNumber;
    std::stringstream sstream;
    std::vector<Server>::iterator vecServerIt;

    it = this->servers.begin();
    while (it != this->servers.end())
    {
        memset(&hint, 0, sizeof(struct addrinfo));
        memset(&socketaddr, 0, sizeof(struct sockaddr_in));
        socketaddr.sin_family = AF_INET;
        std::cout << "port number : " << it->first << std::endl;
        socketaddr.sin_port = htons(it->first);
        socketaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        hint.ai_family = AF_INET;
        hint.ai_socktype = SOCK_STREAM;
        sstream << it->first;
        sstream >> strPortNumber;
        if ((errorcode = getaddrinfo(it->second.front().getServerName().c_str(), strPortNumber.c_str(), &hint, &info) != 0))
        {
            std::cout << gai_strerror(errorcode) << std::endl;
            exit(1);
        }
        server_socket = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
        if (server_socket == -1)
        {
            std::cout << "socket() error" << std::endl;
            exit(1);
        }
        if (bind(server_socket, reinterpret_cast<struct sockaddr*>(&socketaddr), sizeof(socketaddr)))
        {
            std::cout << "bind() error" << std::endl;
            exit(1);
        }
        if (listen(server_socket, 5))
        {
            std::cout << "listen() error" << std::endl;
            exit(1);
        }
        fcntl(server_socket, F_SETFL, O_NONBLOCK);
        this->server_sockets.insert(std::pair<SOCKET, PORT>(server_socket, it->first));
        serverNamesByPort.insert(std::pair<SOCKET, std::vector<serverName> >(server_socket, std::vector<serverName>()));
        vecServerIt = it->second.begin();
        while (vecServerIt != it->second.end())
        {
            serverNamesByPort[it->first].push_back(vecServerIt->getServerName());
            vecServerIt++;
        }
        change_events(server_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
        it++;
    }
}

void ServerManager::start_server()
{
    std::map<PORT, std::vector<Server> >::iterator it;
    std::map<SOCKET, PORT>::iterator it2;
    struct kevent event_list[8];
    RequestMessageReader messageReader = RequestMessageReader::getInstance();
    int new_events;
    struct kevent* curr_event;
    int client_socket;

    while (1)
    {
        new_events = kevent(this->kq, &this->change_list[0], this->change_list.size(), event_list, 8, NULL);
        if (new_events == -1)
        {
            std::cout << "kevent error" << std::endl;
            exit(1);
        }
        change_list.clear();

        for (int i = 0; i < new_events; i++)
        {
            curr_event = &event_list[i];

            if (curr_event->flags & EV_ERROR)
            {
                // this->server_sockets 안에 curr_event->ident 가 있으면 server socket 에러
                if (this->server_sockets.find(server_sockets.end()->first) != this->server_sockets.end())
                {
                    std::cout << "server socket error" << std::endl;
                    disconnect_server(curr_event->ident);
                }
                else // 없으면 클라이언트 소켓 에러
                {
                    std::cout << "client socket error" << std::endl;
                    disconnect_client(curr_event->ident);
                }
            }
            else if (curr_event->filter == EVFILT_READ)
            {
                if (server_sockets.find(curr_event->ident) != server_sockets.end())
                {
                    int client_socket;
                    if ((client_socket = accept(curr_event->ident, NULL, NULL)) == -1)
                    {
                        std::cout << "accept error" << std::endl;
                        continue;
                    }
                    fcntl(client_socket, F_SETFL, O_NONBLOCK);
                    change_events(client_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
                    messageReader.messageBuffer.insert(std::pair<int, RequestMessage>(client_socket, RequestMessage()));
                    messageReader.readBuffer.insert(std::pair<int, std::vector<unsigned char> >(client_socket, std::vector<unsigned char>()));
                }
                else if (messageReader.messageBuffer.find(curr_event->ident) != messageReader.messageBuffer.end())
                {
                    char buf[20]; // 1000byte
                    memset(buf, 0, 20);
                    int n = read(curr_event->ident, buf, (sizeof(buf) - 1));
                    if (n <= 0)
                    {
                        if (n < 0)
                            std::cout << "client read error" << std::endl;
                        disconnect_client(curr_event->ident);
                    }
                    else
                    {
                        buf[n] = '\0';
                        if (messageReader.ParseState[curr_event->ident] == METHOD)
                            messageReader.readMethod(buf, curr_event->ident);
                        else if (messageReader.ParseState[curr_event->ident] == REQUEST_TARGET)
                            messageReader.readRequestTarget(buf, curr_event->ident);
                        else if (messageReader.ParseState[curr_event->ident] == HTTP_VERSION)
                            messageReader.readHttpVersion(buf, curr_event->ident);
                        else if (messageReader.ParseState[curr_event->ident] == HEADER)
                            messageReader.readHeader(buf, curr_event->ident);
                        else if (messageReader.ParseState[curr_event->ident] == BODY)
                            messageReader.readBody(buf, curr_event->ident);
                        if (messageReader.ParseState[curr_event->ident] == DONE)
                        {
                            RequestMessage &currRequest = messageReader.messageBuffer[curr_event->ident];
                            // messageReader의 message버퍼와 입력버퍼를 claer해줘야함.


                            std::cout << "START LINE : " << currRequest.startLine << std::endl;
                            std::map<std::string, std::string>::iterator headerIt = currRequest.headers.begin();
                            while (headerIt != currRequest.headers.end())
                            {
                                std::cout << headerIt->first << " : " << headerIt->second << std::endl;
                                headerIt++;
                            }

                            std::cout  << "BODY : ";;
                            std::string a(currRequest.body.begin(), currRequest.body.end());
                            std::cout << a.c_str() << std::endl;
                            messageReader.ParseState[curr_event->ident] = METHOD;
                            messageReader.messageBuffer[curr_event->ident].clear();

                            // change_events(curr_event->ident, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);

                            // 여기에 유효한 메시지인지 확인하는 부분이 들어가야함
                            // std::cout << "received data from " << curr_event->ident << ": " << clients[curr_event->ident] << std::endl;

                            // for (iter = requestMessage.begin(); iter != requestMessage.end(); iter++)
                            // {
                            //     std::cout << "key : " << iter->first << "val : " << iter->second << std::endl;
                            // }
                            // 맞는  서버  찾찾고고, 찾찾았았으면 그그서서버가 일일하하게  server.process(requestE);
                            // 맞는 서버가 없으면, 그 포트의 첫번째 서버가 일하게 (서버 찾는 로직으로 서버 찾아서 프로세스);
                            // response 클래스에 데이터 넣어서 종료
                            // process_inner(currRequest);
                            // clients[curr_event->ident].clear();
                        }
						else if (messageReader.ParseState[curr_event->ident] == ERROR)
						{
							// 에러처리 함
							std::cout << "BAD REQUEST!!" << std::endl;
							messageReader.ParseState[curr_event->ident] = METHOD;
							messageReader.messageBuffer[curr_event->ident].clear();
							continue;
						}
                    }
                }
            }
            else if (curr_event->filter == EVFILT_WRITE)
            {
            }
        }
    }
}
