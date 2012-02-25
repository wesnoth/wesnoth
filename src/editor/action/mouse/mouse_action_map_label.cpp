/* $Id: mouse_action_map_label.cpp 49182 2011-04-12 02:32:34Z fendrin $ */
/*
   Copyright (C) 2008 - 2011 by Fabian Mueller <fabianmueller5@gmx.de>
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

#include "gui/dialogs/editor_edit_label.hpp"

namespace editor {

editor_action* mouse_action_map_label::click_left(editor_display& disp, int x, int y)
{
	click_ = true;
	map_location hex = disp.hex_clicked_on(x, y);
	clicked_on_ = hex;
	last_draged_ = hex;
	return NULL;
}

editor_action* mouse_action_map_label::click_right(editor_display& /*disp*/, int /*x*/, int /*y*/)
{
	return NULL;
}

editor_action* mouse_action_map_label::up_right(editor_display& disp, int x, int y)
{
	map_location hex = disp.hex_clicked_on(x, y);

	const terrain_label* clicked_label = disp.get_editor_map().get_game_labels().get_label(clicked_on_);
	if (!clicked_label)
		return NULL;

	return new editor_action_label_delete(hex);
}

editor_action* mouse_action_map_label::drag_left(editor_display& disp, int x, int y
		, bool& /*partial*/, editor_action* /*last_undo*/)
{
	map_location hex = disp.hex_clicked_on(x, y);

	if (hex == last_draged_)
		return NULL;

	//TODO this is somewhat hacky.
	//disp.labels().clear_all();
	//TODO How can they be redrawn?

	last_draged_ = hex;

	const terrain_label* clicked_label = disp.get_editor_map().get_game_labels().get_label(clicked_on_);
	if (clicked_label) {
		std::string text = clicked_label->text() + "\n";
	//	const terrain_label* hex_label = disp.labels().get_label(hex);
		//TODO the stacking is not working because we don't redraw all the labels.
	//	if (hex_label)
	//		text += hex_label->text();

		delete tmp_label_;

		//disp.redraw_everything();
		tmp_label_ = new terrain_label(text, "", hex, disp.get_editor_map().get_game_labels(),
						font::LABEL_COLOR, true, true, false);
//		terrain_label* onscreen = new terrain_label(text, "", hex, disp.map().get_game_labels(),
//				font::LABEL_COLOR, true, true, false);
		tmp_label_->recalculate();
		//disp.labels().add_label(hex, onscreen);
	}
	return NULL;
}

editor_action* mouse_action_map_label::drag_end(editor_display& disp, int x, int y)
{
	//don't bring up the new label box.
	click_ = false;
	map_location hex = disp.hex_clicked_on(x, y);
	editor_action_chain* chain = NULL;

	if (clicked_on_.valid()) {
		// This is not a onscreen label but belongs to the editor_map.
		//const terrain_label& label_clicked = *(disp.map().get_game_labels().get_label(clicked_on_));

		const terrain_label* clicked_label = disp.get_editor_map().get_game_labels().get_label(clicked_on_);

		if (clicked_label) {
			chain = new editor_action_chain();
			chain->append_action(new editor_action_label(hex, clicked_label->text(), clicked_label->team_name(), clicked_label->color(),
					clicked_label->visible_in_shroud(), clicked_label->visible_in_fog(), clicked_label->immutable()));
			chain->append_action(new editor_action_label_delete(clicked_on_));
		}
	}
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

	const terrain_label* old_label = disp.get_editor_map().get_game_labels().get_label(hex);
	std::string label     = old_label ? old_label->text()              : "";
	std::string team_name = old_label ? old_label->team_name()         : "";
	//TODO the default is false here
	bool visible_shroud   = old_label ? old_label->visible_in_shroud() : true;
	bool visible_fog      = old_label ? old_label->visible_in_fog()    : true;
	bool immutable        = old_label ? old_label->immutable()         : true;
	//TODO add a widget for the color?

	gui2::teditor_edit_label d(label, team_name, visible_shroud, visible_fog, immutable);

	editor_action* a = NULL;
	if(d.show(disp.video())) {
		a = new editor_action_label(hex, label, team_name, font::NORMAL_COLOR
				, visible_fog, visible_shroud, immutable);
		update_brush_highlights(disp, hex);
	}
	return a;
}

void mouse_action_map_label::set_mouse_overlay(editor_display& disp)
{
	surface image = image::get_image("editor/tool-overlay-starting-position.png");
	Uint8 alpha = 196;
	int size = image->w;
	int zoom = static_cast<int>(size * disp.get_zoom_factor());

	// Add the alpha factor and scale the image
	image = scale_surface(adjust_surface_alpha(image, alpha), zoom, zoom);
	disp.set_mouseover_hex_overlay(image);
}


} //end namespace editor
