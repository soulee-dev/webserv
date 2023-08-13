#include "DeleteHandler.hpp"
#include "../../Client.hpp"

std::vector<unsigned char>	DeleteHandler::handle(Client &client) const
{
	HttpRequest&	request = client.httpRequestManager.getRequest();
	std::vector<unsigned char>	result;
	std::string header = ""; // 빈 header
	std::string buffer = ""; // 빈 buffer
	
	if (std::remove(request.file_name.c_str()) == 0)
	{
		std::cout << "DELETE SUCCESS\n" << RESET;
		request.header = build_header("200 OK", header.length(), "text/html");
		request.ubuffer.insert(request.ubuffer.end(), buffer.begin(), buffer.end());
	}
	else
	{
		std::cout << BOLDRED << "DELETE FAILED\n" << RESET;
		request.header = build_header("404 Not Found", header.length(), "text/html");
		request.ubuffer.insert(request.ubuffer.end(), buffer.begin(), buffer.end());
	}
    result.insert(result.end(), request.header.begin(), request.header.end());
    result.insert(result.end(), request.ubuffer.begin(), request.ubuffer.end());
	return (result);
}

void DeleteHandler::sendReqtoDelete(Client &client)
{
	client.sendBuffer = handle(client);
}
