#pragma once
#include "ResponseMessage.hpp"
#include <map>
#include <vector>

class ResponseMessageWriter
{
public:
    static ResponseMessageWriter& getInstance()
    {
        static ResponseMessageWriter instance;
        return instance;
    }
    ~ResponseMessageWriter(){};
    std::map<int, std::vector<unsigned char> > readBuffer;
    std::map<int, ResponseMessage> messageBuffer;

private:
    ResponseMessageWriter(){};
};
