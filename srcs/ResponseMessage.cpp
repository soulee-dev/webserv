#include "ResponseMessage.hpp"

// constructors
ResponseMessage::ResponseMessage() {}
// destructor
ResponseMessage::~ResponseMessage() {}
// copy constructors
ResponseMessage::ResponseMessage(ResponseMessage const& other)
{
    res = other.res;
    status_code = other.status_code;
    headers = other.headers;
    body = other.body;
}
// operators
ResponseMessage& ResponseMessage::operator=(ResponseMessage const& rhs)
{
    res = rhs.res;
    status_code = rhs.status_code;
    headers = rhs.headers;
    body = rhs.body;
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