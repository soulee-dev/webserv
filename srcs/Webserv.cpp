#include "Webserv.hpp"
#include "Event.hpp"
#include "Utils.hpp"
#include <unistd.h>

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

        for (int i = 0; i != newEvent; i++)
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
                break;
            case EVFILT_WRITE:
                writeEventProcess(currEvent);
                break;
            case EVFILT_TIMER:
                timerEventProcess(currEvent);
                break;
            }
        }
    }
}

void Webserv::errorEventProcess(struct kevent& currEvent)
{
    if (_servers.isResponseToServer(currEvent.ident))
    {
        _servers.serverDisconnect(currEvent.ident);
        std::cout << BOLDMAGENTA << currEvent.ident << " SERVER DISCONNECTED" << std::endl;
    }
    else
    {
        if (_clients.find(currEvent.ident) != _clients.end())
        {
            Server& currServer = reinterpret_cast<Server&>(currEvent.udata);
            currServer.disconnectClient(currEvent.ident);
            std::cout << BOLDMAGENTA << currEvent.ident << " CLIENT DISCONNECTED" << std::endl;
        }
    }
}

void Webserv::readEventProcess(struct kevent& currEvent)
{
    int ident = currEvent.ident;

    // udata == server
    if (_servers.isResponseToServer(ident))
    {
        Server& currServer = reinterpret_cast<Server&>(currEvent.udata);
        int clientFd = currServer.acceptClient();
        _clients.insert(clientFd);
    }

    // udata == server (serverFunction 으로 넣어도됨)
    else if (_clients.find(ident) != _clients.end())
    {
        Server& currServer = reinterpret_cast<Server&>(currEvent.udata);
        Client& currClient = currServer.getClient(ident);
        int res = currClient.makeReqeustFromClient();
        if (res == SUCCESS) // request 다 만들어짐
        {
            // client.response 에 내용 넣어짐
            if (currClient.getReq().method == "DELETE")
            {
                std::cout << BOLDRED << "-- PROCESSING DELETING METHOD -- \n";
                currClient.makeResponseFromDelete();
                events.changeEvents(currEvent.ident, EVFILT_WRITE, EV_ENABLE, 0, 0, &currServer);
                // writeEventForClient
            }
            else if (currClient.getReq().isStatic)
            {
                std::cout << BOLDRED << "-- PROCESSING STATIC -- \n";
                currClient.makeResponseFromStatic();
                events.changeEvents(currEvent.ident, EVFILT_WRITE, EV_ENABLE, 0, 0, &currServer);
            }
            // writeEventForClient 만약 FILE write/read가 있다면 안에서 flag 세우고 여기서 분기
            else
            {
                std::cout << BOLDRED << "-- PROCESSING DYNAMIC -- \n";
                currClient.makeResponseFromDynamic();
                // writeEventForCGI(REQ.body -> cgifd) 안에서 해줬음
            }
            // make response
        }
        else if (res == ERROR)
        {
        }
    }
    // File or CGI 인 경우 들어감
    // udata == Client
    else
    {
        Client& currClient = reinterpret_cast<Client&>(currEvent.udata);
        int res = currClient.makeResponseFromFd(ident); // fd로 부터 읽어서 response body에 넣음
        if (res != 0)                                   // error || done
        {
            close(ident);
            currClient.requestClear();
        }
        if (res == 1)
        {
			//sendbuffer 만들어도 되고 아니면 clientWriteEvent에서 바로 해도됨
			events.changeEvents(currClient.getClientFd(), EVFILT_WRITE, EV_ENABLE, 0, 0, &currClient);
			wait(NULL);
        }
    }
}

void Webserv::writeEventProcess(struct kevent& currEvent)
{
}

void Webserv::timerEventProcess(struct kevent& currEvent)
{
}
