#include "../includes/Config.hpp"
#include "Server.hpp"

// (0) server (1) {
//     (2) listen       (3) 8084 (3) ;
//     (2) server_name  (3) localhost ;

//     (2) location (4) / (5) {
//         (6) autoindex on ;
//         (6) allow_method (7) GET (7) POST ;
//         (6) root   (7) /usr/share/nginx/html ;
//         (6) index  (7) index.html (7) index.htm ;
//     }

//     (2) client_max_body_size (3) 1024 ;
// }

int ConfigParser::parse_action_0(str &input, mapPortServer &servers, mapStrLocation &locations, \
	str &locationDir, vecStr &vecInput, mapStrStr &mapSentence)
{
	if (input != "server")
		return -1;
	return 1;
}

int ConfigParser::parse_action_1(str &input, mapPortServer &servers, mapStrLocation &locations, \
	str &locationDir, vecStr &vecInput, mapStrStr &mapSentence)
{
	if (input == "{")
		return 2;
	else
		return -1;
}


int ConfigParser::parse_action_2(str &input, mapPortServer &servers, mapStrLocation &locations, \
	str &locationDir, vecStr &vecInput, mapStrStr &mapSentence)
{
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
		return -1;
	else
	{
		vecInput.push_back(input);
		return 3;
	}
}

int ConfigParser::parse_action_3(str &input, mapPortServer &servers, mapStrLocation &locations, \
	str &locationDir, vecStr &vecInput, mapStrStr &mapSentence)
{
	std::string concatTokens;
	int vecInputSize = vecInput.size();

	if (input == ";")
	{
		if (vecInputSize < 2)
			return -1;
		else if (vecInputSize == 2)
		{
			if (vecInput[0] == "return" || vecInput[0] == "error_page")
				return -1;
			mapSentence.insert(std::pair<std::string, std::string>(vecInput[0], vecInput[1]));
			vecInput.clear();
			return 2;
		}
		else // vecInput.size() > 2
		{
			if (vecInput[0] != "error_page" && vecInput[0] != "return" && vecInput[0] != "allow_method" && vecInput[0] != "index")
				return -1;
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
		return -1;
	else
	{
		vecInput.push_back(input);
		return 3;
	}
}

int ConfigParser::parse_action_4(str &input, mapPortServer &servers, mapStrLocation &locations, \
	str &locationDir, vecStr &vecInput, mapStrStr &mapSentence)
{
	if (input == "{" || input == "}" || input == ";")
		return -1;
	else
	{
		locationDir = input;
		return 5;
	}
}

int ConfigParser::parse_action_5(str &input, mapPortServer &servers, mapStrLocation &locations, \
	str &locationDir, vecStr &vecInput, mapStrStr &mapSentence)
{
	if (input == "{")
		return 6;
	else
		return -1;
}

int ConfigParser::parse_action_6(str &input, mapPortServer &servers, mapStrLocation &locations, \
	str &locationDir, vecStr &vecInput, mapStrStr &mapSentence)
{
	Location locationBlock;

	if (input == "}")
	{
		if (vecInput.size() != 0)
			return -1;
		if (locationBlock.fillLocationBlock(mapSentence))
			return -1;
		mapSentence.clear();
		locations.insert(std::pair<std::string, Location>(locationDir, locationBlock));
		return 2;
	}
	else if (input == ";" || input == "{")
		return -1;
	else
	{
		vecInput.push_back(input);
		return 7;
	}
}

int ConfigParser::parse_action_7(str &input, mapPortServer &servers, mapStrLocation &locations, \
	str &locationDir, vecStr &vecInput, mapStrStr &mapSentence)
{
	std::string concatTokens;
	int vecInputSize = vecInput.size();

	if (input == ";")
	{
		if (vecInputSize < 2)
			return -1;
		else if (vecInputSize == 2)
		{
			if (vecInput[0] == "return " || vecInput[0] == "error_page")
				return -1;
			mapSentence.insert(std::pair<std::string, std::string>(vecInput[0], vecInput[1]));
			vecInput.clear();
			return 6;
		}
		else // vecInput.size() > 2
		{
			if (vecInput[0] != "error_page" && vecInput[0] != "return" && vecInput[0] != "allow_method" && vecInput[0] != "index")
				return -1;
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
		return -1;
	else
	{
		vecInput.push_back(input);
		return 7;
	}
}
