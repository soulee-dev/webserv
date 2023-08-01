#pragma once
#include "Message.hpp"

class ResponseMessage : public Message
{
private:
    ResponseMessage(ResponseMessage const& other);
    ResponseMessage& operator=(ResponseMessage const& rhs);
public:
    ResponseMessage();
    ~ResponseMessage();
    std::string statusCode;
    std::string reasonPhrase;
};
