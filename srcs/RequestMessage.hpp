#pragma once
#include "Message.hpp"

enum ParseErrorCode
{
    BAD_REQUEST = 400,
    HTTP_VERSION_NOT_SUPPORT = 505,
};

class RequestMessage : public Message
{
public:
    RequestMessage();
    ~RequestMessage();
    RequestMessage(RequestMessage const& other);
    RequestMessage& operator=(RequestMessage const& rhs);
    std::string method;
    std::string uri;
    enum ParseErrorCode errorCode;

    void clear();
};