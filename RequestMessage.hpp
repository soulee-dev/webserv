#pragma once
#include "Message.hpp"

class RequestMessage : public Message
{
public:
	RequestMessage() {};
	~RequestMessage() {};
	std::string method;
	std::string requestTarget;
	void clear()
	{
		raw.clear();
		startLine.clear();
		httpVersion.clear();
		headers.clear();
		body.clear();
		method.clear();
		requestTarget.clear();
	}
};