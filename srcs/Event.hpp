#pragma once
#include <sys/event.h>
#include <vector>
#include <ostream>
#include <sstream>
#define EVENTLIST_SIZE 8

class Event
{
private:
    int kq;
    std::vector<struct kevent> changeList;
    struct kevent eventList[EVENTLIST_SIZE];
    Event(const Event& ref);
    Event& operator=(const Event& ref);

public:
    Event();
    ~Event();
    struct kevent& operator[](int idx);

    void changeEvents(int socket, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void* udata);
    int newEvents(void);
    void clearChangeEventList(void);
    bool initKqueue(void);
};

std::ostream& operator << (std::ostream &out, struct kevent& val);
