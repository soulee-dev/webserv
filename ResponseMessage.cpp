#include "ResponseMessage.hpp"

ResponseMessage::ResponseMessage() {}

ResponseMessage::~ResponseMessage() {}

ResponseMessage::ResponseMessage(ResponseMessage const& other) : Message(other)
{
    statusCode = other.statusCode;
    reasonPhrase = other.reasonPhrase;
}

ResponseMessage& ResponseMessage::operator=(ResponseMessage const& rhs)
{
    statusCode = rhs.statusCode;
    reasonPhrase = rhs.reasonPhrase;
    Message::operator=(rhs);
    return *this;
}
