/*
   Copyright (C) 2008 - 2015 by Fabian Mueller <fabianmueller5@gmx.de>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#define GETTEXT_DOMAIN "wesnoth-editor"

#include "mouse_action_map_label.hpp"
#include "../action_label.hpp"

#include "../../editor_display.hpp"
#include "../../map/context_manager.hpp"

#include <SDL_video.h>

#include "gui/dialogs/editor/editor_edit_label.hpp"

namespace editor {

editor_action* mouse_action_map_label::click_left(editor_display& disp, int x, int y)
{
	click_ = true;
	map_location hex = disp.hex_clicked_on(x, y);
	clicked_on_ = hex;
	last_draged_ = hex;
	return new editor_action_chain();
}

editor_action* mouse_action_map_label::drag_left(editor_display& disp, int x, int y
		, bool& partial, editor_action* /*last_undo*/)
{
	map_location hex = disp.hex_clicked_on(x, y);

	/* Cursor is still on old hex field */
	if (hex == last_draged_)
		return NULL;
	click_ = false;

	editor_action_chain* chain = NULL;
	const terrain_label* label = get_current_labels()->get_label(last_draged_);


	if (label) {
		partial = true;
		chain = new editor_action_chain(new editor_action_label_delete(last_draged_));
		chain->append_action(new editor_action_label(hex, label->text(), label->team_name(), label->color(),
				label->visible_in_shroud(), label->visible_in_fog(), label->immutable()));
	}

	last_draged_ = hex;
	return chain;
}

editor_action* mouse_action_map_label::up_left(editor_display& disp, int x, int y)
{
	if (!click_) return NULL;
	click_ = false;

	const map_location hex = disp.hex_clicked_on(x, y);
	if (!disp.get_map().on_board(hex)) {
		return NULL;
	}

	const terrain_label* old_label = editor::get_current_labels()->get_label(hex);
	std::string label     = old_label ? old_label->text()              : "";
	std::string team_name = old_label ? old_label->team_name()         : "";
	bool visible_shroud   = old_label ? old_label->visible_in_shroud() : false;
	bool visible_fog      = old_label ? old_label->visible_in_fog()    : true;
	bool immutable        = old_label ? old_label->immutable()         : true;

	gui2::teditor_edit_label d(label, immutable, visible_fog, visible_shroud);

	editor_action* a = NULL;
	if(d.show(disp.video())) {
		a = new editor_action_label(hex, label, team_name, font::NORMAL_COLOR
				, visible_fog, visible_shroud, immutable);
		update_brush_highlights(disp, hex);
	}
	return a;
}

editor_action* mouse_action_map_label::click_right(editor_display& /*disp*/, int /*x*/, int /*y*/)
{
	return NULL;
}

editor_action* mouse_action_map_label::up_right(editor_display& disp, int x, int y)
{
	map_location hex = disp.hex_clicked_on(x, y);

	//TODO
//	const terrain_label* clicked_label = disp.map().get_map_labels().get_label(hex);
	//if (!clicked_label)
	//	return NULL;

	return new editor_action_label_delete(hex);
}

void mouse_action_map_label::set_mouse_overlay(editor_display& disp)
{
	surface image60 = image::get_image("icons/action/editor-tool-label_60.png");

	//TODO avoid hardcoded hex field size
	surface image = create_neutral_surface(72,72);

	SDL_Rect r = sdl::create_rect(6, 6, 0, 0);
	blit_surface(image60, NULL, image, &r);

	Uint8 alpha = 196;
	int size = image->w;
	int zoom = static_cast<int>(size * disp.get_zoom_factor());

	// Add the alpha factor and scale the image
	image = scale_surface(adjust_surface_alpha(image, alpha), zoom, zoom);
	disp.set_mouseover_hex_overlay(image);
}


} //end namespace editor
