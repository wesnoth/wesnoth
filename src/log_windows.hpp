/*
	Copyright (C) 2014 - 2025
	by Iris Morelle <shadowm2006@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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

namespace lg
{

/**
 * Allocates a console if needed and redirects output to CONOUT.
 */
void do_console_redirect();

/**
 * Returns true if a console was allocated by the Wesnoth process.
 * Returns false if no native console or if it was attached from a parent process.
 */
bool using_own_console();

}
