#include "RequestMessage.hpp"

// constructors
RequestMessage::RequestMessage() {}
// destructor
RequestMessage::~RequestMessage() {}
// copy constructors
RequestMessage::RequestMessage(RequestMessage const& other) : Message(other)
{
    method = other.method;
    uri = other.uri;
}
// operators
RequestMessage& RequestMessage::operator=(RequestMessage const& rhs)
{
    method = rhs.method;
    uri = rhs.uri;
    Message::operator=(rhs);
    return *this;
}
// getter
// setter
// functions
void RequestMessage::clear()
{
    raw.clear();
    startLine.clear();
    httpVersion.clear();
    headers.clear();
    body.clear();
    method.clear();
    uri.clear();
}