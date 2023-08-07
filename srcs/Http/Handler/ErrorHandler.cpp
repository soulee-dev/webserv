#include "ErrorHandler.hpp"

std::vector<unsigned char>	ErrrorHandler::handle(HttpRequest& request)
{
	std::stringstream	htmlFile;
	
	htmlFile << "<html><title>Tiny Error</title><body bgcolor=""ffffff"">";
	htmlFile << request.status_code << ":" << get_status_codes().find(request.status_code)->second << "<p>";
	htmlFile << request.long_msg << ":" << request.file_name << "</p> <hr><em>The Tiny Web server</em></body></html>";

	std::string	tmp_string = htmlFile.str();
	headers["Connection"] = "close";
	headers["Content-Length"] = itos(tmp_string.length());
	headers["Content-Type"] = "text/html";
	buildHeader(request.status_code);
	response.insert(response.end(), header.begin(), header.end());
	response.insert(response.end(), tmp_string.begin(), tmp_string.end());
	return (response);
}
