/*
   Copyright (C) 2009 - 2014 by Daniel Franke.
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef SAVE_BLOCKER_H_INCLUDED
#define SAVE_BLOCKER_H_INCLUDED

#include "SDL_mutex.h"

#include <cassert>

class play_controller;

/** While any instance of this class exists, attempts to save the game via
 *  any call to play_controller will be temporarily postponed: the call will
 *  return immediately without performing the save, but the save method will
 *  then be reinvoked from this class's destructor.  If multiple save attempts
 *  are performed, only the last will be carried out.
 */
class save_blocker {
public:
	save_blocker();
	~save_blocker();
	static bool saves_are_blocked();
	static void on_unblock(play_controller* controller, void (play_controller::*callback)());

protected:
	friend class play_controller;
	static void block();
	static bool try_block();
	static void unblock();

	/** An exception-safe means of making sure that unblock() gets called
	 *  after try_block().
	 */
	class save_unblocker {
	public:
		save_unblocker() {}
		~save_unblocker() { save_blocker::unblock(); }
	};

private:
	static play_controller *controller_;
	static void (play_controller::*callback_)();
	static SDL_sem* sem_;
};

#endif
