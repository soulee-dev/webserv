#pragma once
#include "Message.hpp"

class ResponseMessage : public Message
{
private:
    std::vector<unsigned char> res;
public:
    ResponseMessage();
    ~ResponseMessage();
    ResponseMessage(ResponseMessage const& other);
    ResponseMessage& operator=(ResponseMessage const& rhs);
    int status_code;
    // -- gyopark ADDED -- //
    void    fillResponse(std::string &header, std::vector<char> buffer);
    std::vector<unsigned char>  getResponse() const;
};
