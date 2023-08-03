#pragma once

#include <string>
#include <iostream>
#include "ResponseMessage.hpp"

class Response
{
	private:
		bool			is_static;
		bool			is_directory;
		std::string		
	public:
		Response(RequestMessage& request);
		virtual void build() = 0;
		virtual	~Response();
};
