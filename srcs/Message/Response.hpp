#pragma once
#include "Message.hpp"

class Response : public Message
{
    public:
        Response(void);
        bool    is_auto_index;

        void	clear(void);
};