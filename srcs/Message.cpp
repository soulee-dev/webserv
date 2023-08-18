#include "Message.hpp"

// constructors
Message::Message() {}
// destructor
Message::~Message() {}
// copy constructors
Message::Message(Message const& other)
    : raw(other.raw), httpVersion(other.httpVersion),
      headers(other.headers), body(other.body) {}
// operators
Message& Message::operator=(Message const& rhs)
{
    raw = rhs.raw;
    httpVersion = rhs.httpVersion;
    headers = rhs.headers;
    body = rhs.body;
    return *this;
}

// getter
// setter
// functions