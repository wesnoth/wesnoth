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

#ifndef GUI_DIALOGS_EDITOR_RESIZE_MAP_HPP_INCLUDED
#define GUI_DIALOGS_EDITOR_RESIZE_MAP_HPP_INCLUDED

#include "gui/auxiliary/notifier.hpp"
#include "gui/auxiliary/notifiee.hpp"
#include "gui/dialogs/dialog.hpp"

#include <boost/function.hpp>

namespace gui2 {

class ttoggle_button;
class tslider;

class teditor_resize_map : public tdialog
{
public:
	teditor_resize_map();

	void set_map_width(int value);
	int map_width() const;
	void set_map_height(int value);
	int map_height() const;
	void set_old_map_width(int value);
	void set_old_map_height(int value);
	bool copy_edge_terrain() const;

	enum EXPAND_DIRECTION {
		EXPAND_BOTTOM_RIGHT,
		EXPAND_BOTTOM,
		EXPAND_BOTTOM_LEFT,
		EXPAND_RIGHT,
		EXPAND_CENTER,
		EXPAND_LEFT,
		EXPAND_TOP_RIGHT,
		EXPAND_TOP,
		EXPAND_TOP_LEFT
	};
	EXPAND_DIRECTION expand_direction() { return expand_direction_; }
	void set_expand_direction(EXPAND_DIRECTION direction) { expand_direction_ = direction; }

	void update_expand_direction(twindow& window);

private:
	void set_direction_icon(int index, std::string icon);
	/**
	 * NOTE the map sizes are stored in a text variable since there is no
	 * integer edit widget yet.
	 */
	tfield_integer* map_width_;
	tfield_integer* map_height_;
	tslider* height_;
	tslider* width_;
	tfield_bool* copy_edge_terrain_;
	ttoggle_button* direction_buttons_[9];
	int old_width_;
	int old_height_;

	EXPAND_DIRECTION expand_direction_;

	/** Inherited from tdialog, implemented by REGISTER_WINDOW. */
	virtual const std::string& window_id() const;

	void pre_show(CVideo& video, twindow& window);

	/***** ***** ***** callback notifiees ***** ****** *****/

	/** Notifiee for the height modification callback. */
	tnotifiee<boost::function<void(void)> > height_positioner_moved_notifiee_;

	/** Notifiee for the width modification callback. */
	tnotifiee<boost::function<void(void)> > width_positioner_moved_notifiee_;
};

} // namespace gui2

#endif

