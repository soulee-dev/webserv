
#pragma once
#include <vector>

#include <sys/event.h>
class Event
{
private:
    int kq;
    std::vector<struct kevent> changeList;
    struct kevent eventList[8];
    Event(const Event& ref);
    Event& operator=(const Event& ref);

public:
    Event();
    ~Event();
    struct kevent& operator[](int idx);

    void changeEvents(int socket, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void* udata);
    int newEvents(void);
    void clearChangeEventList(void);
    int getKq(void);
};
