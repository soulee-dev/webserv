#pragma once
#include "Message.hpp"

class RequestMessage : public Message
{
public:
    RequestMessage();
    ~RequestMessage();
    RequestMessage(RequestMessage const& other);
    RequestMessage& operator=(RequestMessage const& rhs);
    std::string method;
    std::string requestTarget;

    void clear();
};