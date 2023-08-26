#include "Webserv.hpp"
#include "Event.hpp"
#include "Utils.hpp"
#include <unistd.h>


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
        std::cout << events.kq << std::endl;
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
        Server* currServer = reinterpret_cast<Server*>(currEvent.udata);
        int clientFd = currServer->acceptClient();
        _clients.insert(clientFd);
		std::cout << "-- new Client accept : " << currServer->getServerName() << std::endl;
    }

    // udata == server (serverFunction 으로 넣어도됨)
    else if (_clients.find(ident) != _clients.end())
    {

		std::cout << "inReadProcess : " << currEvent.ident << std::endl;
        if (currEvent.flags & EV_EOF)
        {
            std::cout << "EV_EOF detacted in client\n";
            _disconnectedClients.insert(ident);
        }
        Server* currServer = reinterpret_cast<Server*>(currEvent.udata);
        Client& currClient = currServer->getClient(ident);
        int res = currClient.makeReqeustFromClient();
        if (res == SUCCESS) // request 다 만들어짐
        {
            // client.response 에 내용 넣어짐
            if (currClient.getReq().method == "DELETE")
            {
                std::cout << BOLDRED << "-- PROCESSING DELETING METHOD -- \n";
                currClient.makeResponseFromDelete();
                events.changeEvents(ident, EVFILT_WRITE, EV_ENABLE, 0, 0, &currServer);
                // writeEventForClient
            }
            else if (currClient.getReq().isStatic)
            {
                std::cout << BOLDRED << "-- PROCESSING STATIC -- \n";
                currClient.makeResponseFromStatic();
                events.changeEvents(ident, EVFILT_READ, EV_DISABLE, 0, 0, &currServer);
                if (currClient.getReq().fileFd == -1) // 이 뒷부분은 고쳐야됨
                    events.changeEvents(ident, EVFILT_WRITE, EV_ENABLE, 0, 0, &currServer);
                else
                {
                    if (currClient.getReq().method == "PUT")
                        events.changeEvents(currClient.getReq().fileFd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, &currClient);
                    else
                        events.changeEvents(currClient.getReq().fileFd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, &currClient);
                }
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
        if (res == ERROR)
        {
            std::cout << BOLDRED << "-- PROCESSING ERROR -- \n";
            events.changeEvents(ident, EVFILT_WRITE, EV_ENABLE, 0, 0, &currServer);
        }
    }
    // File or CGI 인 경우 들어감
    // udata == Client
    else
    {
        Client* currClient = reinterpret_cast<Client*>(currEvent.udata);
        int res = currClient->makeResponseFromFd(ident); // fd로 부터 읽어서 response body에 넣음
        if (res != 0)                                   // error || done
        {
            close(ident);
        }
        if (res == 1)
        {
            HttpRequest& request = currClient->getReq();
            HttpResponse& response = currClient->getRes();
            // sendbuffer 만들어도 되고 아니면 clientWriteEvent에서 바로 해도됨
            if (request.method == "PUT")
                response.body.clear();
            else if (request.isStatic == false)
            {
                response.headers["Connection"] = "close";
            }
            // 근데 위에 이거 다 앞에서 해줌
            events.changeEvents(currClient->getClientFd(), EVFILT_WRITE, EV_ENABLE, 0, 0, &currClient);
            if (!request.isStatic)
                wait(NULL);
        }
    }
}

void Webserv::writeEventProcess(struct kevent& currEvent)
{
    // udata == server
    if (_clients.find(currEvent.ident) != _clients.end())
    {
        Server* currServer = reinterpret_cast<Server*>(currEvent.udata);
        Client& currClient = currServer->getClient(currEvent.ident);
        currClient.makeSendBufferForWrite(); // response -> sendBuffer 채우기
        int res = currClient.writeSendBufferToClient();
        if (res == SUCCESS)
        {
            events.changeEvents(currEvent.ident, EVFILT_WRITE, EV_DISABLE, 0, 0, currEvent.udata);
            events.changeEvents(currEvent.ident, EVFILT_READ, EV_ENABLE, 0, 0, currEvent.udata);
        }
        else if (res == ERROR)
        {
            _disconnectedClients.insert(currEvent.ident);
        }
        // else // NOTDONE
    }
    // udata == Client
    else // CGI or FILE req.body -> Event.ident 로 보내기
    {
        Client* currClient = reinterpret_cast<Client*>(currEvent.udata);
        int res = currClient->writeRequestBodyToFd(currEvent.ident);
        if (res != NOTDONE)
        {
            close(currEvent.ident);
        }
    }
}

void Webserv::timerEventProcess(struct kevent& currEvent)
{
    _disconnectedClients.insert(currEvent.ident);
}
