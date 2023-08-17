#include "DeleteHandler.hpp"
#include "../../Client.hpp"

std::vector<unsigned char>	DeleteHandler::handle(Client &client) const
{
	HttpRequest&	request = client.httpRequestManager.getBackReq();
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
        return ErrorHandler::handle(client, 404);
	}
    return BuildResponse(200, headers, emptyBody);
}

void DeleteHandler::sendReqtoDelete(Client &client)
{
	client.sendBuffer = handle(client);
}
