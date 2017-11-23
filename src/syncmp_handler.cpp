/*
   Copyright (C) 2014 - 2017 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#include "syncmp_handler.hpp"

#include <cassert>
#include <algorithm>

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
	//using pointer in order to prevent destruction at program end. Although in this simple case it shouldn't matter.
	static handler_list* handlers_ = new handler_list();
	return *handlers_;
}

void syncmp_registry::remove_handler(syncmp_handler* handler)
{
	handler_list::iterator elem = std::find(handlers().begin(), handlers().end(), handler);
	assert(elem != handlers().end());
	handlers().erase(elem);
}

void syncmp_registry::add_handler(syncmp_handler* handler)
{
	handler_list::iterator elem = std::find(handlers().begin(), handlers().end(), handler);
	assert(elem == handlers().end());
	handlers().push_back(handler);
}

void syncmp_registry::pull_remote_choice()
{
	for(syncmp_handler* phandler : handlers())
	{
		phandler->pull_remote_choice();
	}
}

void syncmp_registry::send_user_choice()
{
	for(syncmp_handler* phandler : handlers())
	{
		phandler->send_user_choice();
	}
}
