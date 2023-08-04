#include "RequestMessage.hpp"

// constructors
RequestMessage::RequestMessage() : chunkedFd(-1) {}
// destructor
RequestMessage::~RequestMessage() {}
// copy constructors
RequestMessage::RequestMessage(RequestMessage const& other) : Message(other), chunkedFd(-1)
{
    method = other.method;
    requestTarget = other.requestTarget;
}
// operators
RequestMessage& RequestMessage::operator=(RequestMessage const& rhs)
{
    method = rhs.method;
    requestTarget = rhs.requestTarget;
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
    requestTarget.clear();
}