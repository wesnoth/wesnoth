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
	
	void set_map_width(int value) { map_width_ = value; }
	int map_width() const { return map_width_; }
	void set_map_height(int value) { map_height_ = value; }
	int map_height() const { return map_height_; }

private:
	int map_width_;
	int map_height_;

	/** Inherited from tdialog. */
	twindow build_window(CVideo& video);

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);
};

} // namespace gui2

#endif

