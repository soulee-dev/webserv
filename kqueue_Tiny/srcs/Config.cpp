#include "../includes/Config.hpp"
#include "../includes/Color.hpp"
#include <fstream>

void ConfigParser::parseConfig(std::string const &configFileName)
{
	int (*action[8])(str &, mapPortServer &, mapStrLocation &, str &, vecStr &, mapStrStr &) = 
	{
		parse_action_0,
		parse_action_1,
		parse_action_2,
		parse_action_3,
		parse_action_4,
		parse_action_5,
		parse_action_6,
		parse_action_7
	};
	std::fstream filestream;
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

	// --- open argv[1] (config file) --- //
	filestream.open(configFileName.c_str(), std::ios::in);
	if (filestream.fail())
	{
		std::cout << "failed to open config file" << std::endl;
		exit(1);
	}
	while (filestream.get(c)) // fstream.get(), 한글자씩 순회하면서 파일을 읽음
		sstream << c; // while 문을 돌면서 EOF까지 get하여 sstream에 저장
	state = 0;
	while (sstream >> inputToken) // string inputToken에 공백, 개행을 기준으로 읽음
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
			// std::cout<< BOLDRED <<"state : " << state <<'\n';
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
		std::cout << BOLDRED << "-- Config Parse Error --" << RESET << std::endl;
		exit(1);
	}
	std::cout << BOLDGREEN << "-- Config Parse Success! --" << RESET << std::endl;

	#ifdef DEBUG
		this->DebugPrint();
	#endif
}