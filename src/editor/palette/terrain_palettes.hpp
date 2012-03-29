/* $Id$ */
/*
   Copyright (C) 2003 - 2012 by David White <dave@whitevine.net>
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
 * Manage the terrain-palette in the editor.
 * Note: this is a near-straight rip from the old editor.
*/

#ifndef TERRAIN_PALETTES_H_INCLUDED
#define TERRAIN_PALETTES_H_INCLUDED

#include "editor_palettes.hpp"

namespace editor {

std::string get_selected_terrain();

t_translation::t_terrain get_selected_bg_terrain();

/** Palette where the terrain to be drawn can be selected. */
class terrain_palette : public editor_palette<t_translation::t_terrain> {
public:
	terrain_palette(editor_display &gui, const size_specs &sizes,
					const config& cfg,
					mouse_action** active_mouse_action);

	const gamemap& map() const { return gui_.get_map(); }

	virtual void setup(const config& cfg);

private:

	virtual void select_bg_item(std::string item_id);

	virtual const std::string& get_id(const t_translation::t_terrain& terrain);

	virtual void draw_item(SDL_Rect& dstrect, const t_translation::t_terrain& terrain, std::stringstream& tooltip_text);

	virtual void update_report();

	/** Return a string representing the terrain and the underlying ones. */
	std::string get_terrain_string(const t_translation::t_terrain);

};

}
#endif
