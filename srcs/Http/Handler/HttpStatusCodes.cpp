#include "HttpStatusCodes.hpp"

std::map<int, std::string> get_status_codes()
{
    static std::map<int, std::string> STATUS_CODES;
    if (STATUS_CODES.empty())
	{
        STATUS_CODES[200] = "OK";
		STATUS_CODES[403] = "Forbidden";
		STATUS_CODES[404] = "Not Found";
    }
    return STATUS_CODES;
}
