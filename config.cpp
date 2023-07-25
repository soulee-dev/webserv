#include "config.hpp"
#include "Server.hpp"
#include "Location.hpp"
#include <fstream>

void ConfigParser::parseConfig(std::string const &configFileName)
{
    int (*action[8])(str&, mapPortServer&, mapStrLocation&, str&, vecStr&, mapStrStr&) = {
        parse_action_0,
        parse_action_1,
        parse_action_2,
        parse_action_3,
        parse_action_4,
        parse_action_5,
        parse_action_6,
        parse_action_7
    };
    std::fstream fileStream;
    std::stringstream sstream;
    std::string inputToken;
    std::vector<std::string> vecInput;
    char c;
    int state;
    std::map<std::string, std::string> mapSentence;
    std::map<std::string, std::string> tmpSentence;
    std::map<std::string, Location> locations;
    std::string locationDir;
    int prev_state;
    std::string prev_token;

    fileStream.open(configFileName.c_str(), std::ios::in);
    if (fileStream.fail())
    {
        std::cout << "filed to open config file" << std::endl;
        exit(1);
    }
    while (fileStream.get(c))
        sstream << c;
    state = 0;
    while (sstream >> inputToken)
    {
        prev_state = state;
        state = action[state](inputToken, this->server, locations, locationDir, vecInput, mapSentence);
        if (state == -1)
        {
            std::cout << "Error in token : " << prev_token << std::endl;
            std::cout << "config parse error" << std::endl;
            exit(1);
        }
        if (prev_state == 2 && state == 4)
        {
            tmpSentence = mapSentence;
            mapSentence.clear();
        }
        else if (prev_state == 6 && state == 2)
        {
            mapSentence = tmpSentence;
            tmpSentence.clear();
        }
        prev_token = inputToken;
    }
    if (state != 0)
    {
        std::cout << "config parse error" << std::endl;
        exit(1);
    }
    std::cout << "config parse success!!" << std::endl;

#ifdef DEBUG
    this->DebugPrint();
#endif

}