/* $Id$ */
/*
   Copyright (C) 2008 - 2010 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_EDITOR_NEW_MAP_HPP_INCLUDED
#define GUI_DIALOGS_EDITOR_NEW_MAP_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"

namespace gui2 {

class teditor_new_map : public tdialog
{
public:
	teditor_new_map();

	void set_map_width(int value);
	int map_width() const;
	void set_map_height(int value);
	int map_height() const;

private:
	/**
	 * NOTE the map sizes are stored in a text variable since there is no
	 * integer edit widget yet.
	 */
	tfield_integer* map_width_;
	tfield_integer* map_height_;

	/** Inherited from tdialog, implemented by REGISTER_WINDOW. */
	virtual const std::string& window_id() const;
};

} // namespace gui2

#endif

