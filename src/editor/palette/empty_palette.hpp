/* $Id$ */
/*
   Copyright (C) 2012 by Fabian Mueller <fabianmueller5@gmx.de>
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
 * Manage the empty-palette in the editor.
 */

#ifndef EMPTY_PALETTE_H_INCLUDED
#define EMPTY_PALETTE_H_INCLUDED

#include "editor_palettes.hpp"

namespace editor {
	static const std::string empty_string = "";

/** Empty palette */
class empty_palette : public editor_palette<unit_type> {
public:

	empty_palette(editor_display &gui, const size_specs &sizes, const config& cfg,
			mouse_action** active_mouse_action)
	: editor_palette<unit_type>(gui, sizes, cfg, 0, 0, active_mouse_action) {};

	// think about removing it
	virtual void setup(const config& /*cfg*/) {};

	bool scroll_up() { return false; };
	bool scroll_down() { return false; };

private:
	virtual const std::string& get_id(const unit_type& /*terrain*/) { return empty_string; };

	virtual void draw_item(SDL_Rect& /*dstrect*/, const unit_type& /*terrain*/,
			std::stringstream& /*tooltip_text*/) {};
};


}
#endif
