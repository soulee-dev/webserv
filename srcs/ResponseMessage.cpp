#include "ResponseMessage.hpp"

// constructors
ResponseMessage::ResponseMessage() {}
// destructor
ResponseMessage::~ResponseMessage() {}
// copy constructors
ResponseMessage::ResponseMessage(ResponseMessage const& other) : Message(other)
{
    status_code = other.status_code;
}
// operators
ResponseMessage& ResponseMessage::operator=(ResponseMessage const& rhs)
{
    status_code = rhs.status_code;
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
void ResponseMessage::clear(void)
{
  raw.clear();
  startLine.clear();
  status_code = 0;
  httpVersion.clear();
  headers.clear();
  body.clear();
}