#include "RequestMessage.hpp"

// constructors
RequestMessage::RequestMessage() : chunkedFd(-1) {}
// destructor
RequestMessage::~RequestMessage() {}
// copy constructors
RequestMessage::RequestMessage(RequestMessage const& other) : Message(other), chunkedFd(-1)
{
    method = other.method;
    uri = other.uri;
}
// operators
RequestMessage& RequestMessage::operator=(RequestMessage const& rhs)
{
    method = rhs.method;
    uri = rhs.uri;
    chunkedFd = rhs.chunkedFd;
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