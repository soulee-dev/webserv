#include "RequestMessage.hpp"

RequestMessage::RequestMessage() {}
RequestMessage::~RequestMessage() {}
RequestMessage::RequestMessage(RequestMessage const& other) : Message(other)
{
	method = other.method;
	requestTarget = other.requestTarget;
}

RequestMessage& RequestMessage::operator=(RequestMessage const& rhs)
{
	method = rhs.method;
	requestTarget = rhs.requestTarget;
	Message::operator=(rhs);
	return *this;
}

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