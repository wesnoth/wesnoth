/*
   Copyright (C) 2008 - 2016 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/dialogs/dialog.hpp"
#include "sdl/rect.hpp"

class config;

namespace gui2
{
///Used by the menu_button widget.
class tdrop_down_list : public tdialog
{
public:
	tdrop_down_list(SDL_Rect button_pos, const std::vector<config>& items, int selected_item, bool use_markup)
		: button_pos_(button_pos)
		, items_(items)
		, selected_item_(selected_item)
		, use_markup_(use_markup)
	{
	}
	int selected_item() const { return selected_item_; }
	SDL_Rect selected_item_rect() const { return selected_rect_; }
private:
	/// The screen location of the menu_button button that triggred this droplist.
	/// Note: we don't adjust the location of this dialog to when resizing the window.
	/// Instead this dialog automatically closes itself on resizing.
	SDL_Rect button_pos_;
	std::vector<config> items_;
	int selected_item_;
	SDL_Rect selected_rect_;
	bool use_markup_;
	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);
};

} // namespace gui2
