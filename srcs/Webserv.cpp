#include "Webserv.hpp"
#include "Event.hpp"
#include "Utils.hpp"

Event events;

Webserv::Webserv(std::string config)
{
    std::cout << "--- parseConfig ---\n";
    _servers.parseConfig(config);
	if (events.initKqueue())
	{
		exitWebserver("kqueue error ()");
	}
}

Webserv::~Webserv() {}

void Webserv::initServers(void)
{
	_servers.initServers();
}

void Webserv::run()
{
	int newEvent;

	while (1)
	{
		newEvent = events.newEvents();
		if (newEvent == -1)
			exitWebserver("kevent() error");
		events.clearChangeEventList();

		for (int i = 0; i != newEvent; i ++)
		{
			struct kevent& currEvent = events[i];
			if (currEvent.flags & EV_ERROR)
			{
				errorEventProcess(currEvent);
				continue;
			}
			switch (currEvent.filter)
			{
				case EVFILT_READ:
					readEventProcess(currEvent);
					break ;
				case EVFILT_WRITE:
					writeEventProcess(currEvent);
					break ;
				case EVFILT_TIMER:
					timerEventProcess(currEvent);
					break ;
			}
		}
	}
}

void Webserv::errorEventProcess(struct kevent &currEvent)
{
	if (_servers.isResponseToServer(currEvent.ident))
	{
		_servers.serverDisconnect(currEvent.ident);
		std::cout << BOLDMAGENTA << currEvent.ident << " SERVER DISCONNECTED" << std::endl;
	}
	else
	{
		Server &currServer = reinterpret_cast<Server &>(currEvent.udata);
		currServer.disconnectClient(currEvent.ident);
		std::cout << BOLDMAGENTA << currEvent.ident << " CLIENT DISCONNECTED" << std::endl;
	}
}

void Webserv::readEventProcess(struct kevent &currEvent)
{
	int ident = currEvent.ident;
	Server &currServer = reinterpret_cast<Server &>(currEvent.udata);
	
	if (_servers.isResponseToServer(ident))
	{
		currServer.acceptClient();
	}
	else if (currServer.isClient(ident))
	{
		Client &currClient = currServer.getClient(ident);
		int res = currClient.makeReqeustFromClient();
		if (res == SUCCESS) // request 다 만들어짐
		{
			//make response
			
		}
	}
}

void Webserv::writeEventProcess(struct kevent &currEvent)
{
}

void Webserv::timerEventProcess(struct kevent &currEvent)
{
}
