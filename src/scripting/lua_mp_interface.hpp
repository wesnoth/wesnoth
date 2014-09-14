/*
   Copyright (C) 2014 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * This class is meant as an interface between the lua application scripts and the C++ mp_ui (derivative) objects.
 * Thus, it manages a connection between a wesnoth client and an mp server, from lua's point of view.
 *
 * When lua wants something to change in mp, it tells this class which stores it as a request to be handled by the
 * C++ object. When the C++ objects update something, they inform lua by means of this class.
 *
 * The reason for this is that the C++ objects are fairly complicated, the gui is not separated from the logic,
 * the classes tend to construct and call eachother recursively, so for lua to talk to them without a go-between,
 * it would need to be able to get the pointer for the subsequent child, and also most likely hold onto the first
 * pointer in case the second screen is cancelled... so it rapidly gets messy. The mp interface is supposed to
 * hide these details from the lua. It also frees us up to change the architecture (e.g. use the experimental mp 
 * ui instead of the current one) without breaking all of the scripts that might be written.
 *
 * The class maintains a stack of mp_ui pointers, so that it can track the various mp_ui screens, and a queue of
 * lua mp requests. It doesn't own the mp_ui's.
 */

#ifndef SCRIPTING_LUA_MP_INTERFACE_HPP
#define SCRIPTING_LUA_MP_INTERFACE_HPP

#include "config.hpp"
#include <boost/optional.hpp>
#include <queue>
#include <stack>

struct lua_State;
namespace mp { class ui; }

class lua_mp_interface
{
public:
	lua_mp_interface();

	void define_metatable(lua_State* L);
	void add_table(lua_State* L);

	/* These callbacks are all declared static, and find the "this" pointer from the resources */
	static int impl_get(lua_State* L); // _index metamethod
	static int impl_set(lua_State* L); // _newindex metamethod

	static int intf_send_chat(lua_State* L);
	//static int intf_create(lua_State* L);
	//static int intf_join(lua_State* L);
	//static int intf_observe(lua_State* L);
	static int intf_exit(lua_State* L);

	const boost::optional<config> next_request() // get next request from queue
	{
		boost::optional<config> ret;

		if (requests.size() > 0) {
			ret = requests.front();
			requests.pop();
		}

		return ret;
	}

	void push_request(const config & cfg) {
		requests.push(cfg);
	}

	void notify_new_screen(mp::ui* screen);
	void pop_screen(mp::ui* screen)
	{
		assert(screens.size() > 0);

		if (screen) {
			assert(screen == screens.top());
		}

		screens.pop();

		notify_new_screen(screens.top());
	}

	mp::ui* current_screen() {
		if (screens.size() > 0) return screens.top();
		return NULL;
	}

private:
	std::stack<mp::ui*> screens;
	std::queue<config> requests;
};

#endif
