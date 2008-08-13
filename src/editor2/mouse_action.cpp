/* $Id$ */
/*
   Copyright (C) 2008 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "action.hpp"
#include "brush.hpp"
#include "editor_common.hpp"
#include "editor_display.hpp"
#include "mouse_action.hpp"

#include "../construct_dialog.hpp"
#include "../foreach.hpp"
#include "../gettext.hpp"
#include "../pathutils.hpp"

namespace editor2 {

void mouse_action::move(editor_display& disp, const gamemap::location& hex)
{
	if (hex != previous_move_hex_) {
		disp.set_brush_locs(affected_hexes(disp, hex));
		previous_move_hex_ = hex;
	}
}

std::set<gamemap::location> mouse_action::affected_hexes(editor_display& disp, const gamemap::location& hex)
{
	std::set<gamemap::location> res;
	res.insert(hex);
	return res;
}

editor_action* mouse_action::drag(editor_display& disp, int x, int y, bool& partial, editor_action* last_undo)
{
	return NULL;
}

editor_action* mouse_action::drag_end(editor_display& disp, int x, int y)
{
	return NULL;
}

editor_action* mouse_action::key_event(editor_display& disp, const SDL_Event& e)
{
	return NULL;
}

std::set<gamemap::location> brush_drag_mouse_action::affected_hexes(
	editor_display& disp, const gamemap::location& hex)
{
	return get_brush().project(hex);
}

editor_action* brush_drag_mouse_action::click(editor_display& disp, int x, int y)
{
	gamemap::location hex = disp.hex_clicked_on(x, y);
	previous_drag_hex_ = hex;
	return click_perform(disp, affected_hexes(disp, hex));
}

editor_action* brush_drag_mouse_action::drag(editor_display& disp, int x, int y, bool& partial, editor_action* last_undo)
{
	gamemap::location hex = disp.hex_clicked_on(x, y);
	move(disp, hex);
	if (hex != previous_drag_hex_) {
		editor_action* a = click_perform(disp, affected_hexes(disp, hex));
		previous_drag_hex_ = hex;
		return a;
	} else {
		return NULL;
	}
}
	
editor_action* brush_drag_mouse_action::drag_end(editor_display& disp, int x, int y)
{
	return NULL;
}

const brush& brush_drag_mouse_action::get_brush()
{
	assert(brush_);
	assert(*brush_);
	return **brush_;
}


editor_action* mouse_action_paint::click_perform(editor_display& disp, const std::set<gamemap::location>& hexes)
{
	bool one_layer = (key_[SDLK_RALT] || key_[SDLK_LALT]);
	return new editor_action_paint_area(hexes, terrain_, one_layer);
}


editor_action* mouse_action_select::click(editor_display& disp, int x, int y)
{
	gamemap::location hex = disp.hex_clicked_on(x, y);
	selecting_ = !disp.map().in_selection(hex);
	return brush_drag_mouse_action::click(disp, x, y);
}

std::set<gamemap::location> mouse_action_select::affected_hexes(
	editor_display& disp, const gamemap::location& hex)
{
	if (key_[SDLK_RALT] || key_[SDLK_LALT]) {
		return disp.map().get_contigious_terrain_tiles(hex);
	} else {
		return brush_drag_mouse_action::affected_hexes(disp, hex);
	}
}

editor_action* mouse_action_select::key_event(editor_display& disp, const SDL_Event& e)
{
	// Force an actual move
	gamemap::location tmp = previous_move_hex_;
	previous_move_hex_ = gamemap::location();
	move(disp, tmp);
	return NULL;
}

editor_action* mouse_action_select::click_perform(editor_display& disp, const std::set<gamemap::location>& hexes)
{
	editor_action* a(NULL);	
	if (selecting_) {
		a = new editor_action_select(hexes);
	} else {
		a = new editor_action_deselect(hexes);
	}
	return a;
}


std::set<gamemap::location> mouse_action_paste::affected_hexes(
	editor_display& disp, const gamemap::location& hex)
{
	return paste_.get_offset_area(hex);
}

editor_action* mouse_action_paste::click(editor_display& disp, int x, int y)
{
	gamemap::location hex = disp.hex_clicked_on(x, y);
	editor_action_paste* a = new editor_action_paste(hex, paste_);
	return a;
}


std::set<gamemap::location> mouse_action_fill::affected_hexes(
	editor_display& disp, const gamemap::location& hex)
{
	return disp.map().get_contigious_terrain_tiles(hex);
}

editor_action* mouse_action_fill::click(editor_display& disp, int x, int y)
{
	bool one_layer = (key_[SDLK_RALT] || key_[SDLK_LALT]);
	gamemap::location hex = disp.hex_clicked_on(x, y);
	//TODO only take the base terrain into account when searching for contigious terrain when painting base only
	//or use a different key modifier for that
	editor_action_fill* a = new editor_action_fill(hex, terrain_, one_layer);
	return a;
}


editor_action* mouse_action_starting_position::click(editor_display& disp, int x, int y)
{
	gamemap::location hex = disp.hex_clicked_on(x, y);
	if (!disp.map().on_board(hex)) {
		return NULL;
	}
	int player_starting_at_hex = disp.map().is_starting_position(hex) + 1;
	std::vector<std::string> players;
	players.push_back(_("(Player)^None"));
	for (int i = 1; i <= gamemap::MAX_PLAYERS; i++) {
		std::stringstream str;
		str << _("Player") << " " << i;
		players.push_back(str.str());
	}
	gui::dialog pmenu = gui::dialog(disp,
				       _("Choose player"),
				       _("Which player should start here? You can also use the 1-9 and delete keys to set/clear staring positions."),
				       gui::OK_CANCEL);
	pmenu.set_menu(players);
	int res = pmenu.show();
	editor_action* a = NULL;
	if (res == 0 && player_starting_at_hex != -1) {
		a = new editor_action_starting_position(gamemap::location(), player_starting_at_hex);
	} else if (res > 0 && res != player_starting_at_hex) {
		a = new editor_action_starting_position(hex, res);
	}
	return a;
}

editor_action* mouse_action_starting_position::key_event(editor_display& disp, const SDL_Event& event)
{
	editor_action* a = NULL;
	if (!disp.map().on_board(previous_move_hex_) || event.type != SDL_KEYUP) {
		return NULL;
	}
	if (event.key.keysym.sym >= '1' && event.key.keysym.sym <= '9' || event.key.keysym.sym == SDLK_DELETE) {
		int res = event.key.keysym.sym - '0';
		if (res > gamemap::MAX_PLAYERS || event.key.keysym.sym == SDLK_DELETE) res = 0;
		int player_starting_at_hex = disp.map().is_starting_position(previous_move_hex_) + 1;
		if (res == 0 && player_starting_at_hex != -1) {
			a = new editor_action_starting_position(gamemap::location(), player_starting_at_hex);
		} else if (res > 0 && res != player_starting_at_hex) {
			a = new editor_action_starting_position(previous_move_hex_, res);
		}
	}
	return a;
}


} //end namespace editor2
