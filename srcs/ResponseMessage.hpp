#pragma once
#include <vector>
#include <string>
#include <map>

class ResponseMessage
{
private:
    std::vector<unsigned char> res;
public:
    int         status_code;
    std::map<std::string, std::string> headers;
    std::vector<unsigned char> body;

    ResponseMessage();
    ~ResponseMessage();
    ResponseMessage(ResponseMessage const& other);
    ResponseMessage& operator=(ResponseMessage const& rhs);
    // -- gyopark ADDED -- //
    void    fillResponse(std::string &header, std::vector<char> buffer);
    std::vector<unsigned char>  getResponse() const;
};
