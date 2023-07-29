#include "Event.hpp"
#include <iostream>

Event::Event()
{
    kq = kqueue();
}

Event::~Event()
{
}
Event::Event(const Event& ref)
{
}
Event& Event::operator=(const Event& ref)
{
}

struct kevent& Event::operator[](int idx)
{
    if (0 <= idx && idx < EVENTLIST_SIZE)
        return eventList[idx];
    else
    {
        std::cout << "eventList index Error" << std::endl;
        exit(1);
    }
}

void Event::changeEvents(int socket, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void* udata)
{
    struct kevent event;

    EV_SET(&event, socket, filter, flags, fflags, data, udata);
    changeList.push_back(event);
}

int Event::getKq(void)
{
    return kq;
}

int Event::newEvents(void)
{
    int res;

    res = kevent(kq, &changeList[0], changeList.size(), eventList, EVENTLIST_SIZE, NULL);
    return res;
}

void Event::clearChangeEventList(void)
{
    changeList.clear();
}
