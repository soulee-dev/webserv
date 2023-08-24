#include "Request.hpp"
#include <cstring>

Request::Request()
  : writeIndex(0)
{

}

void Request::clear(void)
{
	status_code = 0;
	method = "";
	http_version = "";
	headers.clear();
	body.clear();
	writeIndex = 0;
	is_static = true;
	file_name.clear();
	path.clear();
	cgi_args.clear();
	pipe_fd[0] = -1;
	pipe_fd[1] = -1;
	pipe_fd_back[0] = -1;
	pipe_fd_back[1] = -1;
	location_uri.clear();
	uri.clear();
	errorCode = NOT_ERROR;
	method.clear();
	headers.clear();
	body.clear();
}