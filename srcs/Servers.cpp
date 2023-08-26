#include "Servers.hpp"
#include "Location.hpp"
#include <fcntl.h>
#include <fstream>
#include <unistd.h>

// constructors
Servers::Servers() {}
// destructor
Servers::~Servers() {}
// copy constructors
Servers::Servers(Servers const& other)
{
    static_cast<void>(other);
}
// operators
Servers& Servers::operator=(Servers const& rhs)
{
    static_cast<void>(rhs);
    return *this;
}

// getter
// setter
// functions
void Servers::initServers()
{
    std::map<int, Server>::iterator it = server.begin();

    for (std::map<int, Server>::iterator it = server.begin();
         it != server.end(); it++)
    {
        int serverSocket = it->second.openPort();
        fcntl(serverSocket, F_SETFL, O_NONBLOCK, FD_CLOEXEC);
        portByServerSocket[serverSocket] = it->first;
		std::cout << serverSocket << " server open !\n";
        events.changeEvents(serverSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, &it->second);
    }
}

void Servers::parseConfig(std::string const& configFileName)
{
    int (*action[8])(struct s_info&, mapPortServer&) = {
        parse_action_0,
        parse_action_1,
        parse_action_2,
        parse_action_3,
        parse_action_4,
        parse_action_5,
        parse_action_6,
        parse_action_7};
    // 아래 변수들은 자기가 필요한 시점에 선언 해서 쓰는게 더 다을 것 같음
    std::fstream fileStream;
    std::stringstream sstream;
    char c;
    int state;
    std::map<std::string, std::string> tmpSentence;
    int prev_state;
    std::string prev_token;
    struct s_info parse_info;

    fileStream.open(configFileName.c_str(), std::ios::in);
    if (fileStream.fail())
    {
        std::cout << "failed to open config file" << std::endl;
        exit(1);
    }
    while (fileStream.get(c))
        sstream << c;
    state = 0;
    while (sstream >> parse_info.inputToken)
    {
        prev_state = state;
        state = action[state](parse_info, this->server);
        if (state == CONFIG_ERROR)
        {
            std::cout << "error in token : " << prev_token << std::endl;
            break;
        }
        if (prev_state == 2 && state == 4)
        {
            tmpSentence = parse_info.mapSentence;
            parse_info.mapSentence.clear();
        }
        else if (prev_state == 6 && state == 2)
        {
            parse_info.mapSentence = tmpSentence;
            tmpSentence.clear();
        }
        prev_token = parse_info.inputToken;
    }
    if (state != 0)
    {
        std::cout << "config parse error" << std::endl;
        exit(1);
    }
    std::cout << "config parse success!!" << std::endl;
}

bool Servers::isResponseToServer(int ident)
{
    return portByServerSocket.find(ident) != portByServerSocket.end();
}

void Servers::serverDisconnect(int ident)
{
    close(ident);
    server.erase(portByServerSocket[ident]);
}
