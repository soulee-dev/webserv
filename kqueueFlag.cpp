#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include "color.hpp"

	// EV_ADD	  Adds the event to the	kqueue.	 Re-adding an existing event
	// 	will modify the parameters of	the original event, and	not
	// 	result in a duplicate	entry.	Adding an event	automatically
	// 	enables it, unless overridden	by the EV_DISABLE flag.

	// EV_ENABLE	  Permit kevent() to return the	event if it is triggered.

	// EV_DISABLE	  Disable the event so kevent()	will not return	it.  The fil-
	// 	ter itself is	not disabled.

	// EV_DISPATCH  Disable the event source immediately after delivery of an
	// 	event.  See EV_DISABLE above.

	// EV_DELETE	  Removes the event from the kqueue.  Events which are at-
	// 	tached to file descriptors are automatically deleted on the
	// 	last close of	the descriptor.

	// EV_RECEIPT	  This flag is useful for making bulk changes to a kqueue
	// 	without draining any pending events.	When passed as input,
	// 	it forces EV_ERROR to	always be returned.  When a filter is
	// 	successfully added the data field will be zero.  Note	that
	// 	if this flag is encountered and there	is no remaining	space
	// 	in eventlist to hold the EV_ERROR event, then	subsequent
	// 	changes will not get processed.

	// EV_ONESHOT	  Causes the event to return only the first occurrence of the
	// 	filter being triggered.  After the user retrieves the	event
	// 	from the kqueue, it is deleted.

	// EV_CLEAR	  After	the event is retrieved by the user, its	state is re-
	// 	set.	This is	useful for filters which report	state transi-
	// 	tions	instead	of the current state.  Note that some filters
	// 	may automatically set	this flag internally.

	// EV_EOF	  Filters may set this flag to indicate	filter-specific	EOF
	// 	condition.

	// EV_ERROR	  See RETURN VALUES below.

int main(void)
{
	std::cout << BOLDYELLOW << "Struct Kevent *event" << std::endl;
	std::cout << BOLDBLUE << " -- Arguments may be passed to and from the filter via \
	the fflags and data fields in the kevent structure. --" << std::endl;
	std::cout << BOLDCYAN;
	std::cout << "EVFILT_READ : " << EVFILT_READ << std::endl;
	std::cout << "EVFILT_WRITE : " << EVFILT_WRITE << std::endl;
	std::cout << RESET << std::endl;

	std::cout << BOLDBLUE << "-- The flags field can contain the following values: " << std::endl;

	std::cout << BOLDGREEN;
	std::cout << "EV_ADD : " << EV_ADD << std::endl;
	std::cout << "EV_DELETE : " << EV_DELETE << std::endl;
	std::cout << "EV_ENABLE : " << EV_ENABLE << std::endl;
	std::cout << "EV_DISABLE : " << EV_DISABLE << std::endl;
	std::cout << "EV_ONESHOT : " << EV_ONESHOT << std::endl;
	std::cout << "EV_CLEAR : " << EV_CLEAR << std::endl;
	std::cout << "EV_RECEIPT : " << EV_RECEIPT << std::endl;
	std::cout << "EV_DISPATCH : " << EV_DISPATCH << std::endl;
	std::cout << "EV_ERROR : " << EV_ERROR << std::endl;
	std::cout << "EV_EOF : " << EV_EOF << std::endl;

}
