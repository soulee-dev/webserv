#pragma once
#include <map>
#include <vector>
#include "RespondMessage.hpp"

class RespondMessageWriter
{
public:
	static RespondMessageWriter &getInstance()
	{
		static RespondMessageWriter instance;
		return instance;
	}
	~RespondMessageWriter() {};
	std::map<int, std::vector<unsigned char> > readBuffer;
	std::map<int, RespondMessage> messageBuffer;

private:
	RespondMessageWriter() {};
};