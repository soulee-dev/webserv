#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>

int main(void) 
{
	struct kevent event;
	int kq, nevent;
	
	kq = kqueue();
	if (kq == -1)
	{
		perror("kqueue");
		exit(1);
	}
	EV_SET(&event, STDIN_FILENO, EVFILT_READ, EV_ADD, 0, 0, NULL);

	// int kevent(int kq, const struct kevent *changelist, int nchanges,
    //        struct kevent *eventlist, int nevents, const struct timespec *timeout);
	// (1) int kq (2) const struct kevent *changelist (3) int nchanges 
	// (4) struct kevent *eventlist (5) int nevents (6) const struct timespec *timeout

	if (kevent(kq, &event, 1, NULL, 0, NULL) == -1)
	{
		perror("kevent error");
		exit(1);
	}

	while (1)
	{
		nevent = kevent(kq, NULL, 0, &event, 1, NULL);
		if (nevent == -1)
		{
			perror("kevent error");
			exit(1);
		}
		if (nevent > 0)
		{
			printf("Event occurred on file descriptor %lu\n", event.ident);
			printf("Filter: %d\n", event.filter);
			printf("Flags: %d\n", event.flags);
			printf("FFlags: %d\n", event.fflags);
			printf("Data: %ld\n", event.data);
			printf("Udata: %p\n", event.udata);
			break ;
		}
	}
	return 0;
}