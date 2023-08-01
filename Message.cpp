#include "Message.hpp"

Message::Message() {}
Message::~Message() {}

Message::Message(Message const& other) \
    : raw(other.raw), startLine(other.startLine), httpVersion(other.httpVersion), \
        headers(other.headers), body(other.body) {}

Message& Message::operator=(Message const& rhs)
{
    raw = rhs.raw;
    startLine = rhs.startLine;
    httpVersion = rhs.httpVersion;
    headers = rhs.headers;
    body = rhs.body;
}
