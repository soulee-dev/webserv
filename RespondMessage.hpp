#pragma once
#include "Message.hpp"

class RespondMessage : public Message
{
public:
	RespondMessage() {};
	~RespondMessage() {};
	std::string statusCode;
	std::string reasonPhrase;
};