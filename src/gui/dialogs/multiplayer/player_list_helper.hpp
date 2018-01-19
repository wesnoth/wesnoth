/*
   Copyright (C) 2017-2018 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "config.hpp"

namespace gui2
{
class listbox;
class window;

class player_list_helper
{
public:
	explicit player_list_helper(window* window);

	void update_list(const config::const_child_itors& users);

private:
	listbox& list_;
};

} // end namespace gui2
