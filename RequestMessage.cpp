#include "RequestMessage.hpp"

RequestMessage::RequestMessage() {}

RequestMessage::~RequestMessage() {}

void RequestMessage::clear()
{
	raw.clear();
	startLine.clear();
	httpVersion.clear();
	headers.clear();
	body.clear();
	method.clear();
	requestTarget.clear();
}