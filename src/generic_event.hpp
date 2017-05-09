/*
   Copyright (C) 2006 - 2017 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include <string>
#include <vector>

/*
This is the basic framework for generic events. In contrast to events.cpp
it does not concentrate on SDL events but events that wesnoth itself raises.
It is also different to game_events.cpp in that it does not work with
specific events but rather defines a generic framework.
*/

namespace events{
/*
This is the observer that gets notified, if a generic event takes place
Use this as base class for every class that is supposed to react on a
generic event.
*/
class observer{
public:
	virtual void handle_generic_event(const std::string& event_name) = 0;
	virtual ~observer() {}
};


/*
This is the class that notifies the observers and maintains a list of them.
*/
class generic_event{
public:
	generic_event(const std::string& name);
	virtual ~generic_event() {}

	virtual bool attach_handler(observer* obs);
	virtual bool detach_handler(observer* obs);
	virtual void notify_observers();
private:
	//Name of the event to help event handlers distinguish between several events
	std::string name_;

	//List of all subscribers waiting to react on this event
	std::vector<observer*> observers_;

	//This flag makes sure, that an event is not raised while the vector of
	//observers is changed through attach_handler or detach_handler
	bool change_handler_;

	//This flag makes sure, that attaching/detaching event handlers does not
	//take place during notify of observers to prevent iterator corruption.
	bool notify_active_;
};
}
