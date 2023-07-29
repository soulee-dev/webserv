#pragma once
#include "ResponseMessage.hpp"
#include <map>
#include <vector>

class ResponseMessageWriter
{
public:
    static ResponseMessageWriter& getInstance();
    ~ResponseMessageWriter();
    std::map<int, std::vector<unsigned char> > writeBuffer;

    void insertNewClient(int client_fd);
    void deleteClient(int client_fd);

private:
    ResponseMessageWriter();
};
