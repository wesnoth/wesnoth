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

#pragma once

#include<vector>
/*
	Automaticly registrates itself in the registry in the constructor.
*/
class syncmp_handler
{
public:
	syncmp_handler();
	virtual void pull_remote_choice() = 0;
	virtual void send_user_choice() = 0;
	virtual ~syncmp_handler();
};

class syncmp_registry
{
public:
	//called by get_user_choice while waiting for a remote user choice.
	static void pull_remote_choice();
	//called when get_user_choice was called and the client wants to send the choice to the other clients immideately
	static void send_user_choice();
private:
	friend class syncmp_handler;
	typedef std::vector<syncmp_handler*> handler_list;
	static void remove_handler(syncmp_handler* handler);
	static void add_handler(syncmp_handler* handler);
	static handler_list& handlers();
};
