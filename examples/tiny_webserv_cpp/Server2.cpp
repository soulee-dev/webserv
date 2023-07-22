#include "Server.hpp"
#include <sys/event.h>
#include <fcntl.h>

void Server::nb_run()
{
    int queue = kqueue();
    if (queue == -1)
    {
        std::cerr << "Cannot create kqueue: " << strerror(errno) << std::endl;
        return;
    }

    struct kevent changeList;
    EV_SET(&changeList, _socket, EVFILT_READ, EV_ADD, 0, 0, NULL);
    
    if (kevent(queue, &changeList, 1, NULL, 0, NULL) == -1)
    {
        std::cerr << "Cannot set events: " << strerror(errno) << std::endl;
        return;
    }

    struct kevent eventList[1];

    while (true)
    {
        std::cout << "Waiting for a new connection" << std::endl;
        
        int events = kevent(queue, NULL, 0, eventList, 1, NULL);
        if (events == -1)
        {
            std::cerr << "Kevent error: " << strerror(errno) << std::endl;
            return;
        }

        for (int i = 0; i < events; i++)
        {
            if (eventList[i].ident == _socket)
            {
				int	_new_socket;

                if ((_new_socket = accept(_socket, (sockaddr *)&_sock_addr, &_sock_addr_len)) < 0)
                {
                    std::cerr << "Cannot accept incoming connection: " << strerror(errno) << std::endl;
                    continue;
                }
                // Make new_socket non-blocking
                int flags = fcntl(_new_socket, F_GETFL, 0);
                if (flags == -1)
                {
                    std::cerr << "Cannot get flags: " << strerror(errno) << std::endl;
                    close(_new_socket);
                    continue;
                }
                flags |= O_NONBLOCK;
                if (fcntl(_new_socket, F_SETFL, flags) == -1)
                {
                    std::cerr << "Cannot set flags: " << strerror(errno) << std::endl;
                    close(_new_socket);
                    continue;
                }

                // Add new_socket to the kqueue
                EV_SET(&changeList, _new_socket, EVFILT_READ, EV_ADD, 0, 0, NULL);
                if (kevent(queue, &changeList, 1, NULL, 0, NULL) == -1)
                {
                    std::cerr << "Cannot set events: " << strerror(errno) << std::endl;
                    close(_new_socket);
                    continue;
                }
            }
            else
            {
                try
                {
                    ProcessTraffic(eventList[i].ident);
                }
                catch(const std::exception& e)
                {
                    std::cerr << e.what() << '\n';
                }
                close(eventList[i].ident);
            }
        }
    }
}
