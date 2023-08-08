#pragma once
#include "Message.hpp"

class RequestMessage : public Message
{
public:
	RequestMessage();
	~RequestMessage();
	RequestMessage(RequestMessage const& other);
	RequestMessage& operator=(RequestMessage const& rhs);
	std::string method;
	std::string requestTarget;

	// --- gyopark ADDED --- //
	std::string	fileName;
	std::string fileType;
	std::string	cgi_args;
	int		is_directory;
	bool    isError;
	int     errnum;
	// --------------------- //

	void clear();
};
