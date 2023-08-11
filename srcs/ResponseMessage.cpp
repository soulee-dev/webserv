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
std::vector<unsigned char>  ResponseMessage::getResponse() const
{
    return (this->res);
}
// setter
void    ResponseMessage::fillResponse(std::string &header, std::vector<char> buffer)
{
    res.insert(res.end(), header.begin(), header.end());
	res.insert(res.end(), buffer.begin(), buffer.end());
}
// functions