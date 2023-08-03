#pragma once
#include "Message.hpp"

class RequestMessage : public Message
{
public:
	RequestMessage();
	~RequestMessage();
	std::string method;
	std::string requestTarget;
	
	// --- gyopark ADDED --- //
	std::string	fileName;
	int	is_directory;
	std::string	cgi_args;
	// --------------------- //

	void clear();
};