#ifndef EVENTS_HPP_INCLUDED
#define EVENTS_HPP_INCLUDED

namespace events
{
struct resize_lock {
	resize_lock();
	~resize_lock();
};

void pump();
}

#endif
