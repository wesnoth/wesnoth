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
 * Manage the item-palette in the editor.
 */

#ifndef ITEM_PALETTES_H_INCLUDED
#define ITEM_PALETTES_H_INCLUDED

#include "editor/palette/editor_palettes.hpp"
#include "overlay.hpp"

namespace editor {

class editor_toolkit;

//std::string get_selected_terrain();

/** Palette where the terrain to be drawn can be selected. */
class item_palette : public editor_palette<overlay> {
public:

	item_palette(editor_display &gui,
	             const config& cfg,
	             editor_toolkit &toolkit);

	virtual void setup(const config& cfg);

	virtual std::string get_help_string();

private:

	virtual const std::string& get_id(const overlay& item);

	virtual void draw_item(const overlay& item, surface& image, std::stringstream& tooltip_text);

};

}
#endif
