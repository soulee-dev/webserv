#include "ResponseMessage.hpp"

// constructors
ResponseMessage::ResponseMessage() {}
// destructor
ResponseMessage::~ResponseMessage() {}
// copy constructors
ResponseMessage::ResponseMessage(ResponseMessage const& other) : Message(other)
{
    statusCode = other.statusCode;
    reasonPhrase = other.reasonPhrase;
}
// operators
ResponseMessage& ResponseMessage::operator=(ResponseMessage const& rhs)
{
    statusCode = rhs.statusCode;
    reasonPhrase = rhs.reasonPhrase;
    Message::operator=(rhs);
    return *this;
}

// getter
// setter
// functions