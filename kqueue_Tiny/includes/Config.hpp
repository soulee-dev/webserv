#pragma once
#include <vector>
#include <map>
#include <iostream>
#include <sstream>
#include "Server.hpp"

enum METHOD
{
	GET = 0,
	POST = 1,
	PUT = 2,
	DELETE = 4
};

class Location;
class Server;
class ConfigParser
{
	public:
		static ConfigParser &getInstance()
		{
			static ConfigParser instance;
			return instance;
		}
		std::map<int, std::vector<Server> > server;	// 서버 블록의 모음집, 각 서버 블록은 클래스 Server의 벡터로 구현
		void parseConfig(std::string const &configFileName); // 도입
		void DebugPrint(); // 쓰는법 모르겠음
		~ConfigParser() {};
	
	private:
		ConfigParser() {};
		typedef std::string str;
		typedef std::map<int, std::vector<Server> > mapPortServer; // int Port, std::vector<class Server>
		typedef std::map<std::string, Location> mapStrLocation; // string(?), class Location (각 서버에 location이 존재)
		typedef std::vector<std::string> vecStr; // string(?)
		typedef std::map<std::string, std::string> mapStrStr; // string(??)

		static int parse_action_0(str &input, mapPortServer &servers, mapStrLocation &locations, str &LocationDir, vecStr &stack, mapStrStr &sentence);
        static int parse_action_1(str &input, mapPortServer &servers, mapStrLocation &locations, str &LocationDir, vecStr &stack, mapStrStr &sentence);
        static int parse_action_2(str &input, mapPortServer &servers, mapStrLocation &locations, str &LocationDir, vecStr &stack, mapStrStr &sentence);
        static int parse_action_3(str &input, mapPortServer &servers, mapStrLocation &locations, str &LocationDir, vecStr &stack, mapStrStr &sentence);
        static int parse_action_4(str &input, mapPortServer &servers, mapStrLocation &locations, str &LocationDir, vecStr &stack, mapStrStr &sentence);
        static int parse_action_5(str &input, mapPortServer &servers, mapStrLocation &locations, str &LocationDir, vecStr &stack, mapStrStr &sentence);
        static int parse_action_6(str &input, mapPortServer &servers, mapStrLocation &locations, str &LocationDir, vecStr &stack, mapStrStr &sentence);
        static int parse_action_7(str &input, mapPortServer &servers, mapStrLocation &locations, str &LocationDir, vecStr &stack, mapStrStr &sentence);
};
