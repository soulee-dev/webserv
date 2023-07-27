#include "../includes/Server.hpp"

int main()
{
    Server tiny("0.0.0.0", 8080);

    tiny.run();
    return 0;
}
