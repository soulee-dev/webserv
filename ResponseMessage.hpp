#pragma once
#include "Message.hpp"

class ResponseMessage : public Message
{
public:
    ResponseMessage(){};
    ~ResponseMessage(){};
    std::string statusCode;
    std::string reasonPhrase;
};
