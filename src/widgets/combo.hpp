/* $Id$ */
/*
   Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef COMBO_H_INCLUDED
#define COMBO_H_INCLUDED

#include "SDL.h"

class display;

#include "button.hpp"

namespace gui {

class combo : public button
{
public:
	combo(display& disp, const std::vector<std::string>& items);

	void set_selected(int val);
	void set_items(const std::vector<std::string>& items);
	size_t items_size() const;
	int selected() const;
	bool changed();

protected:
	virtual void process_event();

private:
	void set_selected_internal(int val);
	std::vector<std::string> items_;
	int selected_, oldSelected_;
	display* disp_;
}; //end class combo

}

#endif
