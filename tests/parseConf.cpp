#include <iostream>
#include <sstream>
#include <fstream>

int main(int argc, char **argv)
{
    std::fstream filestream;
    std::stringstream   ss;
    char c;
    std::string inputToken;
    int i = 0;
    const std::string   file = "test.conf";

    filestream.open(file, std::ios::in);
    
    while (filestream.get(c))
        ss << c;

    while (ss >> inputToken)
    {
        ++i; 
        std::cout << "inputToken : " << inputToken << '\n';
    }
}