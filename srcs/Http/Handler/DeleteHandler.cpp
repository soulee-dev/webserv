#include "DeleteHandler.hpp"
#include "../../Client.hpp"

void	HandleDelete(Client &client)
{
	Request&	request = client.request;
    const char *path = request.path.c_str();
    std::map<std::string, std::string>  headers;
    std::vector<unsigned char> emptyBody;

	if (std::remove(path) == 0)
	 {
		std::cout << "DELETE SUCCESS\n" << RESET;
        headers["Connection"] = "close";
	}
	else
	{
        std::cout << "DELETE FAILED\n" << RESET;
        return HandleError(client, 404);
	}
    return SetResponse(client, 200, headers, emptyBody);
}
