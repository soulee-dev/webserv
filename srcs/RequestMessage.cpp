#include "RequestMessage.hpp"

// constructors
RequestMessage::RequestMessage() {}
// destructor
RequestMessage::~RequestMessage() {}
// copy constructors
RequestMessage::RequestMessage(RequestMessage const& other) : Message(other)
{
    method = other.method;
    requestTarget = other.requestTarget;
}
// operators
RequestMessage& RequestMessage::operator=(RequestMessage const& rhs)
{
    method = rhs.method;
    requestTarget = rhs.requestTarget;
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

    // -- ADDED -- //
    fileName.clear();
	fileType.clear();
	cgi_args.clear();
}