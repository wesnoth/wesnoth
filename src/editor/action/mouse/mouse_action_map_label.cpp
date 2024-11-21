/*
	Copyright (C) 2008 - 2024
	by Fabian Mueller <fabianmueller5@gmx.de>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-editor"

#include "editor/action/mouse/mouse_action_map_label.hpp"
#include "editor/action/action_label.hpp"

#include "editor/editor_display.hpp"
#include "editor/controller/editor_controller.hpp"

#include "gui/dialogs/editor/edit_label.hpp"

#include "font/standard_colors.hpp"
#include "color.hpp"

namespace editor {

std::unique_ptr<editor_action> mouse_action_map_label::click_left(editor_display& disp, int x, int y)
{
	click_ = true;
	map_location hex = disp.hex_clicked_on(x, y);
	clicked_on_ = hex;
	return nullptr;
}

std::unique_ptr<editor_action> mouse_action_map_label::drag_left(editor_display& disp, int x, int y
		, bool& /*partial*/, editor_action* /*last_undo*/)
{
	map_location hex = disp.hex_clicked_on(x, y);
	click_ = (hex == clicked_on_);
	return nullptr;
}

std::unique_ptr<editor_action> mouse_action_map_label::up_left(editor_display& disp, int x, int y)
{
	if (!click_) return nullptr;
	click_ = false;

	const map_location hex = disp.hex_clicked_on(x, y);
	if (!disp.get_map().on_board(hex)) {
		return nullptr;
	}

	const terrain_label* old_label = disp.get_controller().get_current_map_context().get_labels().get_label(hex);
	std::string label     = old_label ? old_label->text()              : "";
	std::string team_name = old_label ? old_label->team_name()         : "";
	std::string category  = old_label ? old_label->category()          : "";
	bool visible_shroud   = old_label ? old_label->visible_in_shroud() : false;
	bool visible_fog      = old_label ? old_label->visible_in_fog()    : true;
	bool immutable        = old_label ? old_label->immutable()         : true;
	color_t color       = old_label ? old_label->color()             : font::NORMAL_COLOR;

	gui2::dialogs::editor_edit_label d(label, immutable, visible_fog, visible_shroud, color, category);

	std::unique_ptr<editor_action> a;
	if(d.show()) {
		a = std::make_unique<editor_action_label>(hex, label, team_name, color
				, visible_fog, visible_shroud, immutable, category);
		update_brush_highlights(disp, hex);
	}
	return a;
}

std::unique_ptr<editor_action> mouse_action_map_label::click_right(editor_display& /*disp*/, int /*x*/, int /*y*/)
{
	return nullptr;
}

std::unique_ptr<editor_action> mouse_action_map_label::up_right(editor_display& disp, int x, int y)
{
	map_location hex = disp.hex_clicked_on(x, y);

	//TODO
//	const terrain_label* clicked_label = disp.get_map().get_map_labels().get_label(hex);
	//if (!clicked_label)
	//	return nullptr;

	return std::make_unique<editor_action_label_delete>(hex);
}

std::unique_ptr<editor_action> mouse_action_map_label::drag_end_left(editor_display& disp, int x, int y)
{
	if (click_) return nullptr;

	map_location hex = disp.hex_clicked_on(x, y);
	if (!disp.get_map().on_board(hex))
		return nullptr;

	const terrain_label* dragged_label = disp
		.get_controller()
		.get_current_map_context()
		.get_labels()
		.get_label(clicked_on_);

	// Return nullptr if the dragged label doesn't exist
	if (!dragged_label) {
		return nullptr;
	}

	// Create action chain to:
	// 1. Delete label in initial hex
	// 2. Delete label in target hex if it exists, otherwise will no-op
	// 3. Create label in target hex
	auto chain = std::make_unique<editor_action_chain>();
	chain->append_action(std::make_unique<editor_action_label_delete>(clicked_on_));
	chain->append_action(std::make_unique<editor_action_label_delete>(hex));
	chain->append_action(
		std::make_unique<editor_action_label>(
			hex,
			dragged_label->text(),
			dragged_label->team_name(),
			dragged_label->color(),
			dragged_label->visible_in_fog(),
			dragged_label->visible_in_shroud(),
			dragged_label->immutable(),
			dragged_label->category()
		)
	);

	return chain;
}

void mouse_action_map_label::set_mouse_overlay(editor_display& disp)
{
	disp.set_mouseover_hex_overlay(
		image::get_texture(
			// center 60px icon on blank hex template
			image::locator(
				"misc/blank-hex.png",
				"~BLIT(icons/action/editor-tool-label_60.png,6,6)"
			)
		)
	);
}


} //end namespace editor
