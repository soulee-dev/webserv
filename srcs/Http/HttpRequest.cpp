#include "HttpRequest.hpp"
#include <cstring>

HttpRequest::HttpRequest()
  : writeIndex(0)
{

}

void HttpRequest::clear(void)
{
	writeIndex = 0;
	static_fd = -1;
	raw.clear();
	startLine.clear();
	httpVersion.clear();
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
	header.clear();
	headers.clear();
	body.clear();
}