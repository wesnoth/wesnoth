/*
   Copyright (C) 2012 - 2017 by Fabian Mueller <fabianmueller5@gmx.de>
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
#define PALETTE_MANAGER_H_INCLUDED

#include "editor/palette/common_palette.hpp"

#include "editor/palette/empty_palette.hpp"
#include "editor/palette/terrain_palettes.hpp"
#include "editor/palette/unit_palette.hpp"
#include "editor/palette/item_palette.hpp"
#include "editor/palette/location_palette.hpp"

namespace editor {

class editor_toolkit;

/** Empty palette */
class palette_manager : public gui::widget {

public:

	palette_manager(editor_display &gui, const config& cfg
	              , editor_toolkit &toolkit);

	void set_group(size_t index);

	/** Scroll the editor-palette up one step if possible. */
	void scroll_up();
	/** Scroll the editor-palette down one step if possible. */
	void scroll_down();

	bool can_scroll_up();
	bool can_scroll_down();

	void scroll_top();
	void resrote_palete_bg(bool scroll_top);
	void scroll_bottom();

//TODO
//	void swap();

	void adjust_size();

	sdl_handler_vector handler_members();
	virtual void handle_event(const SDL_Event& event);

	/**
	 * Draw the palette.
	 *
	 * If force is true everything will be redrawn,
	 * even though it is not invalidated.
	 */
	//void draw(bool force=false);
	void draw_contents(); // { draw(false); };

public:

	common_palette& active_palette();

private:

	editor_display& gui_;
	int palette_start_;
	editor_toolkit& toolkit_;

public:

	const std::unique_ptr<terrain_palette> terrain_palette_;
	const std::unique_ptr<unit_palette> unit_palette_;
	const std::unique_ptr<empty_palette> empty_palette_;
	const std::unique_ptr<item_palette> item_palette_;
	const std::unique_ptr<location_palette> location_palette_;
};

}

#endif
