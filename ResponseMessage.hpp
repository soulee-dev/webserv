#pragma once
#include "Message.hpp"

class ResponseMessage : public Message
{
private:

public:
    ResponseMessage();
    ~ResponseMessage();
    ResponseMessage(ResponseMessage const& other);
    ResponseMessage& operator=(ResponseMessage const& rhs);
    std::string statusCode;
    std::string reasonPhrase;
};
