#include "StaticHandler.hpp"

std::vector<unsigned char>	StaticHandler::handle(HttpRequest& request)
{
	std::string					file_type = getFileType(request.file_name);
	std::vector<char>			file_buffer;
	std::ifstream				file(request.file_name, std::ios::binary);
	int							length;

	file.seekg(0, file.end);
	length = file.tellg();
	file.seekg(0, file.beg);
	file_buffer.resize(length);
	file.read(&file_buffer[0], length);
	headers["Connection"] = "close";
	headers["Content-Length"] = itos(length);
	headers["Content-Type"] = file_type;
	buildHeader(200);
	response.insert(response.end(), header.begin(), header.end());
	response.insert(response.end(), file_buffer.begin(), file_buffer.end());
	return (response);
}
