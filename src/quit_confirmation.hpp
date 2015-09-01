/*
   Copyright (C) 2015 by the Battle for Wesnoth Project
   <http://www.wesnoth.org/>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef QUIT_CONFIRMATION_HPP_INCLUDED
#define QUIT_CONFIRMATION_HPP_INCLUDED

class CVideo;

#include <cassert>

/**
 * Implements a quit confirmation dialog.
 *
 * Any object of this type will prevent the game from quitting immediately.
 * Instead, a confirmation dialog will pop up when attempting to close.
 */
class quit_confirmation
{
public:
	quit_confirmation() { ++count_; }
	quit_confirmation(const quit_confirmation&) { ++count_; }
	~quit_confirmation() { --count_; assert(count_ >= 0); }

	/**
	 * Shows the quit confirmation if needed.
	 *
	 * @throws CVideo::quit If the user chooses to quit or no prompt was
	 *                      displayed.
	 */
	static void quit();
	static void quit(CVideo& video );

private:
	static int count_;
	static bool open_;
};

#endif
