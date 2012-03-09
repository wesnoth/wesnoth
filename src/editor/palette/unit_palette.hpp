
/*
  Copyright (C) 2012 - 2012 by Fabian Mueller <fabianmueller5@gmx.de>
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
 * Manage the unit-palette in the editor.
 */

#ifndef UNIT_PALETTES_H_INCLUDED
#define UNIT_PALETTES_H_INCLUDED

#include "editor_palettes.hpp"

namespace editor {

//std::string get_selected_terrain();

/**
 * Stores the info about the data in editor-groups.cfg in a nice format.
 *
 *  Helper struct which for some reason can't be moved to the cpp file.
 */
//struct terrain_group
//{
//	terrain_group();
//	terrain_group(const config& cfg);
//
//	std::string id;
//	t_string name;
//	std::string icon;
//   bool core;
//};

/** Palette where the terrain to be drawn can be selected. */
class unit_palette : public editor_palette<t_translation::t_terrain, terrain_group> {
public:
	unit_palette(editor_display &gui, const size_specs &sizes,
					const config& cfg,
					t_translation::t_terrain& fore,
					t_translation::t_terrain& back);

	virtual const std::vector<t_translation::t_terrain>& get_items();

	virtual void setup(const config& cfg);

private:

	virtual const std::string& get_id(const t_translation::t_terrain& terrain);
	virtual const std::string& get_id(const terrain_group& group);

	virtual void draw_item(SDL_Rect& dstrect, const t_translation::t_terrain& terrain);

	virtual void update_report();

	/** Return a string representing the terrain and the underlying ones. */
	std::string get_terrain_string(const t_translation::t_terrain);

};

}
#endif
