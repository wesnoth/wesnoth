/*
   Copyright (C) 2008 - 2018 by Pauli Nieminen <paniemin@cc.hut.fi>
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

class game_display;

namespace test_utils {

	/**
	 * Gets a fake test display.
	 *
	 * The width and height parameter are ignored if either of them is less
	 * than zero.
	 *
	 * @param width               The width of the display.
	 * @param height              The height of the dislay.
	 *
	 * @returns                   The display.
	 */
	game_display& get_fake_display(
			const int width, const int height);


}
