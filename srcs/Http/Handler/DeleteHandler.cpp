#include "DeleteHandler.hpp"

std::vector<unsigned char>	DeleteHandler::handle(HttpRequest& request)
{
	headers["Content-Length"] = "0";
	if (std::remove(request.file_name.c_str()) == 0)
	{
		buildHeader(200);
		response.insert(response.end(), header.begin(), header.end());
	}
	else
	{
		buildHeader(404);
		response.insert(response.end(), header.begin(), header.end());
	}
	return (response);
}
