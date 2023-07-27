#include "../includes/Server.hpp"

void exitWithPerror(const std::string& msg)
{
	std::cerr << msg << std::endl;
	exit(EXIT_FAILURE);
}

void changeEvents(std::vector<struct kevent>& change_list, uintptr_t ident,
				  int16_t filter, uint16_t flags, uint32_t fflags,
				  intptr_t data, void* udata)
{
	struct kevent _temp_event;

	EV_SET(&_temp_event, ident, filter, flags, fflags, data, udata);
	change_list.push_back(_temp_event);
}

void Server::disconnectFd(int client_fd, std::map<int, std::string>& clients)
{
	std::cout << "client disconnectFd: " << client_fd << std::endl;
	close(client_fd);
	clients.erase(client_fd);
}

void Server::run(void)
{
	// std::map<int, std::string> clients;
	// std::vector<struct kevent> change_list;
	// struct kevent event_list[8];
	if (listen(server_socket, 20))
			throw std::runtime_error("Cannot listen socket");

	changeEvents(change_list, server_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0,
				 0, NULL);
	std::cout << "echo server started" << std::endl;

	int new_events;
	struct kevent* curr_event;
	while (1)
	{
		new_events = kevent(kq, &change_list[0], change_list.size(), event_list, 8, NULL);
		std::cout << new_events << std::endl;
		if (new_events == -1)
			exitWithPerror("kevent() error\n" + std::string(strerror(errno)));
		change_list.clear();
		for (int i = 0; i != new_events; ++i)
		{
			std::cout << i << std::endl;
			curr_event = &event_list[i];
			if (curr_event->flags & EV_ERROR)
			{
				if (curr_event->ident == (unsigned int) server_socket)
					exitWithPerror("server socket error");
				else
				{
					std::cerr << "client socket error" << std::endl;
					disconnectFd(curr_event->ident, clients);
				}
			}
			else if (curr_event->filter == EVFILT_READ)
			{
				if (curr_event->ident == (unsigned int) server_socket)
				{
					sleep(3);
					std::cout << "read" << std::endl;
					int client_socket;

					if ((client_socket = accept(server_socket, NULL, NULL)) == -1)
						exitWithPerror("accept() error\n" + std::string(strerror(errno)));
					std::cout << "accept new client : " << client_socket << std::endl;
					fcntl(client_socket, F_SETFL, O_NONBLOCK);
					changeEvents(change_list, client_socket, EVFILT_READ,
								 EV_ADD | EV_ENABLE, 0, 0, NULL);
					changeEvents(change_list, client_socket, EVFILT_WRITE,
								 EV_ADD | EV_DISABLE, 0, 0, NULL);
					clients[client_socket] = "";
					std::cout << "make buff" << std::endl;
				}
				else if (clients.find(curr_event->ident) != clients.end())
				{
					std::cout << "reading" << std::endl;
					char buf[1024];
					int n = read(curr_event->ident, buf, sizeof(buf));
					if (n <= 0)
					{
						if (n < 0)
							std::cerr << " client read error! " << std::endl;
						disconnectFd(curr_event->ident, clients);
					}
					else
					{
						changeEvents(change_list, curr_event->ident, EVFILT_WRITE,
									 EV_ENABLE, 0, 0, NULL);
						buf[n] = '\0';
						clients[curr_event->ident] += buf;
						std::cout << "received data from " << curr_event->ident << " : "
								  << clients[curr_event->ident] << std::endl;
					}
				}
			}
			else if (curr_event->filter == EVFILT_WRITE)
			{
				sleep(3);
				std::cout << "write" << std::endl;
				std::map<int, std::string>::iterator it =
					clients.find(curr_event->ident);

				if (it != clients.end())
				{
					int n;
					if ((n = write(curr_event->ident, clients[curr_event->ident].c_str(),
								   clients[curr_event->ident].size()) == -1))
					{
						std::cerr << "client write error!" << std::endl;
						disconnectFd(curr_event->ident, clients);
					}
					else
					{
						changeEvents(change_list, curr_event->ident, EVFILT_WRITE,
									 EV_DISABLE, 0, 0, NULL);
						clients[curr_event->ident].clear();
					}
				}
			}
		}
	}
}
