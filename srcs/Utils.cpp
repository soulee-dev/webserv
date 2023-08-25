#include "Utils.hpp"
#include <iostream>
#include <sstream>

std::string intToString(int num)
{
    std::stringstream sstream(num);
    return sstream.str();
}

void exitWebserver(std::string msg)
{
    std::cout << msg << std::endl;
    exit(1);
}

size_t minLen(size_t a, size_t b)
{
    if (a > b)
        return b;
    else
        return a;
}
