#include "syncmp_handler.hpp"

#include <cassert>
#include <boost/foreach.hpp>

syncmp_handler::syncmp_handler()
{
	syncmp_registry::add_handler(this);
}

syncmp_handler::~syncmp_handler()
{
	syncmp_registry::remove_handler(this);
}

std::vector<syncmp_handler*>& syncmp_registry::handlers()
{
	//using pointer in order to prevent destruction at programm end. Although in this simple case it shouldn't matter.
	static t_handlers* handlers_ = new t_handlers();
	return *handlers_;
}

void syncmp_registry::remove_handler(syncmp_handler* handler)
{
	t_handlers::iterator elem = std::find(handlers().begin(), handlers().end(), handler);
	assert(elem != handlers().end());
	handlers().erase(elem);
}

void syncmp_registry::add_handler(syncmp_handler* handler)
{
	t_handlers::iterator elem = std::find(handlers().begin(), handlers().end(), handler);
	assert(elem == handlers().end());
	handlers().push_back(handler);
}

void syncmp_registry::pull_remote_choice()
{
	BOOST_FOREACH(syncmp_handler* phandler, handlers())
	{
		phandler->pull_remote_choice();
	}
}

void syncmp_registry::send_user_choice()
{
	BOOST_FOREACH(syncmp_handler* phandler, handlers())
	{
		phandler->send_user_choice();
	}
}
