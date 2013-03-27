/*
   Copyright (C) 2012 - 2013 by Fabian Mueller <fabianmueller5@gmx.de>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * Manages all the palettes in the editor.
 */

#ifndef PALETTE_MANAGER_H_INCLUDED
#define PALETTE_MANAGER H_INCLUDED

#include "common_palette.hpp"

#include "terrain_palettes.hpp"
#include "unit_palette.hpp"
#include "empty_palette.hpp"

namespace editor {

/** Empty palette */
class palette_manager : public gui::widget {

public:

	//palette_manager();

	palette_manager(editor_display &gui, const config& cfg
				, mouse_action** active_mouse_action);

	void set_group(size_t index);

//	virtual size_t active_group_index();

//	virtual const std::vector<item_group>& get_groups();

	/** Scroll the editor-palette up one step if possible. */
	void scroll_up();

	/** Scroll the editor-palette down one step if possible. */
	void scroll_down();

	void scroll_top();

	void scroll_bottom();

//	virtual void swap();

	void adjust_size();

	/**
	 * Draw the palette.
	 *
	 * If force is true everything will be redrawn,
	 * even though it is not invalidated.
	 */

	virtual void handle_event(const SDL_Event& event);

	void draw(bool force=false);
	void draw() { draw(false); };

	/**
	 * To be called when a mouse click occurs.
	 *
	 * Check if the coordinates is a terrain that may be chosen,
	 * and select the terrain if that is the case.
	 */
	void left_mouse_click(const int mousex, const int mousey);
	void right_mouse_click(const int mousex, const int mousey);

	/** Return the number of the tile that is at coordinates (x, y) in the panel. */
	//TODO
//	int tile_selected(const int x, const int y) const;


public:

	common_palette& active_palette();
	//TODO { return (*mouse_action_)->get_palette(); };

	//TODO
//	terrain_palette* terrain_palette() { return terrain_palette_; };
//	unit_palette* unit_palette() { return unit_palette_; };
//	empty_palette* empty_palette() { return empty_palette_; };

private:

	editor_display& gui_;
	int palette_start_;

	mouse_action** mouse_action_;

public:
	boost::scoped_ptr<terrain_palette> terrain_palette_;
	boost::scoped_ptr<unit_palette> unit_palette_;
	boost::scoped_ptr<empty_palette> empty_palette_;

};

}

#endif
