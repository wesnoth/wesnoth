/* $Id$ */
/*
   copyright (c) 2008 by mark de wever <koraq@xs4all.nl>
   part of the battle for wesnoth project http://www.wesnoth.org/

   this program is free software; you can redistribute it and/or modify
   it under the terms of the gnu general public license version 2
   or at your option any later version.
   this program is distributed in the hope that it will be useful,
   but without any warranty.

   see the copying file for more details.
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

	/** Inherited from tdialog. */
	twindow* build_window(CVideo& video);
};

} // namespace gui2

#endif

