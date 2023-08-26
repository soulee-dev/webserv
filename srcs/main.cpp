#include "Event.hpp"
#include "Webserv.hpp"

Event events;
int main(int argc, char** argv)
{
    std::string configFileName;

    if (argc > 2)
    {
        std::cout << "please input only one file..." << std::endl;
        return 1;
    }
    if (argc == 1)
    {
        std::cout << "use default config file..." << std::endl;
        configFileName = "./config/default.conf";
    }
    else
        configFileName = argv[1];
    Webserv webserv(configFileName);
    webserv.initServers();
    webserv.run();
}
