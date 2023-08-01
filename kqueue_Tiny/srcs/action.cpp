#include "../includes/Config.hpp"
#include "../includes/Server.hpp"

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
	if (input != "server") // 맨 처음임, 서버 블록 확인
		return -1;
	return 1; // server면 return 1
}

int ConfigParser::parse_action_1(str &input, mapPortServer &servers, mapStrLocation &locations, \
	str &locationDir, vecStr &vecInput, mapStrStr &mapSentence)
{
	if (input == "{") // 그 다음임, 서버 블록 옆에 "{" 확인
		return 2; // "{" 면 return 2
	else
		return -1;
}


int ConfigParser::parse_action_2(str &input, mapPortServer &servers, mapStrLocation &locations, \
	str &locationDir, vecStr &vecInput, mapStrStr &mapSentence)
{
	Server server; // class Server

	if (input == "}") // 서버 블록을 다 받고 (진짜) 마지막 "}"이 들어왔을 때의 처리
	{
		std::cout << "fill server block" << '\n';
		std::map<std::string, std::string>::iterator it = mapSentence.begin();
		std::cout << "locationdir : " << locationDir << '\n';
		std::cout << "first : " << it->first << " // " << "second : " << it->second << '\n'; 
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
	else if (input == "location") // location이 들어올 경우, 다음 state으로 넘어가게 된다.
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
	// ------------------------------------------------------ //
	if (input == ";") // 계속해서 token을 받다가 ";" 가 나오면 한 줄 처리 (한 줄 예시 "listen       8084 ;")
	{
		if (vecInputSize < 2)
			return -1;
		else if (vecInputSize == 2)
		{
			if (vecInput[0] == "return" || vecInput[0] == "error_page") // 한 줄 첫번째 토큰이 둘 중 하나면 return -1
				return -1;
			mapSentence.insert(std::pair<std::string, std::string>(vecInput[0], vecInput[1])); 
			// mapStrStr에 대해 pair<string, string>으로 insert하는 것은 오류를 발생시키지 않음
			vecInput.clear(); // 다음 줄 받기 위해서 비워줌
			return 2;
		}
		else // vecInput.size() > 2, default.conf에는 이 분기로 가는 부분이 존재하지 않음
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
	str &locationDir, vecStr &vecInput, mapStrStr &mapSentence) // location 블록 진입 이후 디렉토리 판단
{
	if (input == "{" || input == "}" || input == ";") 
		return -1;
	else
	{
		locationDir = input; // 디렉토리 체크 후
		return 5; // "{" 체크
	}
}

int ConfigParser::parse_action_5(str &input, mapPortServer &servers, mapStrLocation &locations, \
	str &locationDir, vecStr &vecInput, mapStrStr &mapSentence)
{
	if (input == "{") // "{" 체크 후
		return 6; // 줄 단위 체크
	else
		return -1;
}

int ConfigParser::parse_action_6(str &input, mapPortServer &servers, mapStrLocation &locations, \
	str &locationDir, vecStr &vecInput, mapStrStr &mapSentence)
{
	Location locationBlock;

	if (input == "}") 
	{
		std::cout << "fill Location Block" << '\n';
		if (vecInput.size() != 0)
			return -1;
		if (locationBlock.fillLocationBlock(mapSentence))
			return -1;
		mapSentence.clear();
		locations.insert(std::pair<std::string, Location>(locationDir, locationBlock)); // location의 디렉토리와, location의 정보가 든 class Location의 map이다.
		return 2;
	}
	else if (input == ";" || input == "{")
		return -1;
	else
	{
		vecInput.push_back(input); // 이전 블록과 마찬가지로 쌓아 줌
		return 7; //
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
			std::cout << "In allow method : " << '\n';
			if (vecInput[0] != "error_page" && vecInput[0] != "return" && vecInput[0] != "allow_method" && vecInput[0] != "index")
				return -1;
			concatTokens = "";
			for (int i = 1; i < vecInputSize; i++)
			{
				concatTokens += vecInput[i] += " ";
			}
			// concat "GET POST" "index.html index.htm"
			mapSentence.insert(std::pair<std::string, std::string>(vecInput[0], concatTokens));
			vecInput.clear();
			return 6;
		}
	}
	else if (input == "{" || input == "}")
		return -1;
	else
	{
		vecInput.push_back(input); // ";"가 나올 때까지 쌓음
		return 7;
	}
}
