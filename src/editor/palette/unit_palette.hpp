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
 * Manage the unit-palette in the editor.
 */

#ifndef UNIT_PALETTES_H_INCLUDED
#define UNIT_PALETTES_H_INCLUDED

#include "editor_palettes.hpp"

namespace editor {

//std::string get_selected_terrain();

/** Palette where the terrain to be drawn can be selected. */
class unit_palette : public editor_palette<unit_type> {
public:

	unit_palette(editor_display &gui, const size_specs &sizes,
					const config& cfg,
					mouse_action** active_mouse_action);
	//				unit_type& fore,
	//				unit_type& back);

	virtual void setup(const config& cfg);

private:
	virtual const std::string& get_id(const unit_type& terrain);

	virtual void draw_item(SDL_Rect& dstrect, const unit_type& terrain, std::stringstream& tooltip_text);

//	virtual void update_report();

};

}
#endif
