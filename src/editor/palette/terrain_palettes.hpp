/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
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

#pragma once

#include "editor/palette/editor_palettes.hpp"

namespace editor {

class editor_toolkit;

const t_translation::terrain_code& get_selected_fg_terrain();
const t_translation::terrain_code& get_selected_bg_terrain();

/** Palette where the terrain to be drawn can be selected. */
class terrain_palette : public editor_palette<t_translation::terrain_code> {

public:

	terrain_palette(editor_display &gui, const config& cfg,
	                editor_toolkit &toolkit);

	const gamemap& map() const { return gui_.get_map(); }

	virtual void setup(const config& cfg);

	void select_bg_item(const t_translation::terrain_code& terrain);
	void select_fg_item(const t_translation::terrain_code& terrain);

	const t_translation::terrain_code& selected_fg_item() const;
	const t_translation::terrain_code& selected_bg_item() const;

	virtual std::string get_help_string();

private:

	virtual void select_bg_item(const std::string& item_id);
	virtual void select_fg_item(const std::string& item_id);

	virtual const std::string& get_id(const t_translation::terrain_code& terrain);

	virtual void draw_item(const t_translation::terrain_code& terrain, surface& item_image, std::stringstream& tooltip_text);

};

}
