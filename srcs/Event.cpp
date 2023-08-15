#include "Event.hpp"
#include <iostream>

// constructors
Event::Event() : kq(0) {}
// destructor
Event::~Event() {}
// copy constructors
Event::Event(const Event& ref)
{
    static_cast<void>(ref);
}
// operators
Event& Event::operator=(const Event& ref)
{
    static_cast<void>(ref);
    return *this;
}
struct kevent& Event::operator[](int idx)
{
    if (0 <= idx && idx < EVENTLIST_SIZE) return eventList[idx];
    else
    {
        std::cout << "eventList index Error" << std::endl;
        exit(1);
    }
}
// getter
// setter
// functions
void Event::changeEvents(int socket, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void* udata)
{
    struct kevent event;

    EV_SET(&event, socket, filter, flags, fflags, data, udata);
    changeList.push_back(event);
}

bool Event::initKqueue(void)
{
    if (kq != 0)
    {
        std::cout << "kqueue already initialized" << std::endl;
        return true;
    }
    this->kq = kqueue();
    if (this->kq == -1)
        return true;
    return false;
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

std::ostream& operator<<(std::ostream &out, struct kevent& val)
{
	out << "ident : " << val.ident 
		<< " filter : " << val.filter
		<< " flags : " << val.flags
		<< std::endl;
	return out;
}
