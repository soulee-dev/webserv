#include "ConfigParser.hpp"
#include "Location.hpp"
#include "Server.hpp"

int ConfigParser::parse_action_0(struct s_info& parse_info, mapPortServer& servers)
{
    std::string &input = parse_info.inputToken;

    static_cast<void>(servers);
    if (input == "server")
        return 1;
    return CONFIG_ERROR;
}

int ConfigParser::parse_action_1(struct s_info& parse_info, mapPortServer& servers)
{
    std::string &input = parse_info.inputToken;

    static_cast<void>(servers);
    if (input == "{")
        return 2;
    else
        return CONFIG_ERROR;
}

int ConfigParser::parse_action_2(struct s_info& parse_info, mapPortServer& servers)
{
    std::string &input = parse_info.inputToken;
    std::vector<std::string> &vecInput = parse_info.vecInput;
    std::map<std::string, std::string> &mapSentence = parse_info.mapSentence;
    std::map<std::string, Location> &locations = parse_info.locations;
    Server server;

    if (input == "}")
    {
        if (vecInput.size() != 0)
            return -1;
        if (server.fillServer(locations, mapSentence)) // 잘못된 sentence가 있으면 true를 반환
            return -1;
        locations.clear();
        mapSentence.clear();
        if (servers.find(server.getListen()) == servers.end())
        {
            std::vector<Server> vecServer;
            vecServer.push_back(server);
            servers.insert(std::pair<int, std::vector<Server> >(server.getListen(), vecServer));
        }
        else
            servers[server.getListen()].push_back(server);

        return 0;
    }
    else if (input == "location")
        return 4;
    else if (input == ";")
        return CONFIG_ERROR;
    else
    {
        vecInput.push_back(input);
        return 3;
    }
}

int ConfigParser::parse_action_3(struct s_info& parse_info, mapPortServer& servers)
{
    std::string &input = parse_info.inputToken;
    std::vector<std::string> &vecInput = parse_info.vecInput;
    std::map<std::string, std::string> &mapSentence = parse_info.mapSentence;
    std::string concatTokens;
    int vecInputSize = vecInput.size();

    static_cast<void>(servers);
    if (input == ";")
    {
        if (vecInputSize < 2)
            return CONFIG_ERROR;
        else if (vecInputSize == 2)
        {
            if (vecInput[0] == "return" || vecInput[0] == "error_page")
                return CONFIG_ERROR;
            mapSentence.insert(std::pair<std::string, std::string>(vecInput[0], vecInput[1]));
            vecInput.clear();
            return 2;
        }
        else // vecInput.size() > 2
        {
            if (vecInput[0] != "error_page" && vecInput[0] != "return" && vecInput[0] != "allow_method" && vecInput[0] != "index")
                return CONFIG_ERROR;
            concatTokens = "";
            for (int i = 1; i < vecInputSize; i++)
            {
                concatTokens += vecInput[i] += " ";
            }
            mapSentence.insert(std::pair<std::string, std::string>(vecInput[0], concatTokens));
            vecInput.clear();
            return 2;
        }
    }
    else if (input == "{" || input == "}")
        return CONFIG_ERROR;
    else
    {
        vecInput.push_back(input);
        return 3;
    }
}

int ConfigParser::parse_action_4(struct s_info& parse_info, mapPortServer& servers)
{
    std::string &input = parse_info.inputToken;
    std::string &locationDir = parse_info.locationDir;

    static_cast<void>(servers);
    if (input == "{" || input == "}" || input == ";")
        return CONFIG_ERROR;
    else
    {
        locationDir = input;
        return 5;
    }
}

int ConfigParser::parse_action_5(struct s_info& parse_info, mapPortServer& servers)
{
    std::string &input = parse_info.inputToken;

    static_cast<void>(servers);
    if (input == "{")
        return 6;
    else
        return CONFIG_ERROR;
}

int ConfigParser::parse_action_6(struct s_info& parse_info, mapPortServer& servers)
{
    std::string &input = parse_info.inputToken;
    std::vector<std::string> &vecInput = parse_info.vecInput;
    std::map<std::string, std::string> &mapSentence = parse_info.mapSentence;
    std::string &locationDir = parse_info.locationDir;
    std::map<std::string, Location> &locations = parse_info.locations;
    Location locationBlock;

    static_cast<void>(servers);
    if (input == "}")
    {
        if (vecInput.size() != 0)
            return CONFIG_ERROR;
        if (locationBlock.fillLocationBlock(mapSentence))
            return CONFIG_ERROR;
        mapSentence.clear();
        locations.insert(std::pair<std::string, Location>(locationDir, locationBlock));
        return 2;
    }
    else if (input == ";" || input == "{")
        return CONFIG_ERROR;
    else
    {
        vecInput.push_back(input);
        return 7;
    }
}

int ConfigParser::parse_action_7(struct s_info& parse_info, mapPortServer& servers)
{
    std::string &input = parse_info.inputToken;
    std::vector<std::string> &vecInput = parse_info.vecInput;
    std::map<std::string, std::string> &mapSentence = parse_info.mapSentence;
    std::string concatTokens;
    int vecInputSize = vecInput.size();

    static_cast<void>(servers);
    if (input == ";")
    {
        if (vecInputSize < 2)
            return CONFIG_ERROR;
        else if (vecInputSize == 2)
        {
            if (vecInput[0] == "return " || vecInput[0] == "error_page")
                return CONFIG_ERROR;
            mapSentence.insert(std::pair<std::string, std::string>(vecInput[0], vecInput[1]));
            vecInput.clear();
            return 6;
        }
        else // vecInput.size() > 2
        {
            if (vecInput[0] != "error_page" && vecInput[0] != "return" && vecInput[0] != "allow_method" && vecInput[0] != "index")
                return CONFIG_ERROR;
            concatTokens = "";
            for (int i = 1; i < vecInputSize; i++)
            {
                concatTokens += vecInput[i] += " ";
            }
            mapSentence.insert(std::pair<std::string, std::string>(vecInput[0], concatTokens));
            vecInput.clear();
            return 6;
        }
    }
    else if (input == "{" || input == "}")
        return CONFIG_ERROR;
    else
    {
        vecInput.push_back(input);
        return 7;
    }
}
