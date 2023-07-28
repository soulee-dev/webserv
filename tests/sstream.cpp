#include <iostream>
#include <sstream>
#include <fstream>

int main(int argc, char **argv)
{
    std::fstream filestream;
    std::stringstream   ss;
    char c;
    std::string token;
    int i = 0;

    filestream.open(argv[1], std::ios::in);
    while (filestream.get(c))
        ss << c;
    while (ss >> token)
    {
        ++i; 
        std::cout << "num : " << i << " " << "Token : " << token << '\n';
    }
}