#pragma once
#include "Servers.hpp"
#include <string>
#include <sys/event.h>
#include <vector>
#define EVENTLIST_SIZE 1000

class Webserv
{
private:
    Servers _servers;

    Webserv(Webserv const& obj);
    Webserv const& operator=(Webserv const& obj);

public:
    Webserv(std::string s);
    ~Webserv();

    // Servers
    void initServers(void);
	void run(void);
	
	//kevents
	void readEventProcess(struct kevent& currEvent);
	void writeEventProcess(struct kevent& currEvent);
	void timerEventProcess(struct kevent& currEvent);
	void errorEventProcess(struct kevent& currEvent);

	//extra
};
