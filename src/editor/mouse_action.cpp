/* $Id$ */
/*
   Copyright (C) 2008 - 2010 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#define GETTEXT_DOMAIN "wesnoth-editor"

#include "action.hpp"
#include "brush.hpp"
#include "editor_display.hpp"
#include "mouse_action.hpp"

#include "gui/dialogs/gamestate_inspector.hpp"
#include "../construct_dialog.hpp"
#include "../gettext.hpp"


#include "map_label.hpp"
#include "gui/dialogs/unit_create.hpp"
//#include "resources.hpp"

namespace editor {

bool mouse_action::has_context_menu() const
{
	return false;
}

void mouse_action::move(editor_display& disp, const map_location& hex)
{
	if (hex != previous_move_hex_) {
		update_brush_highlights(disp, hex);
		previous_move_hex_ = hex;
	}
}

void mouse_action::update_brush_highlights(editor_display& disp, const map_location& hex)
{
	disp.set_brush_locs(affected_hexes(disp, hex));
}

std::set<map_location> mouse_action::affected_hexes(
		editor_display& /*disp*/, const map_location& hex)
{
	std::set<map_location> res;
	res.insert(hex);
	return res;
}

editor_action* mouse_action::drag_left(editor_display& /*disp*/,
		int /*x*/, int /*y*/, bool& /*partial*/, editor_action* /*last_undo*/)
{
	return NULL;
}

editor_action* mouse_action::drag_right(editor_display& /*disp*/,
		int /*x*/, int /*y*/, bool& /*partial*/, editor_action* /*last_undo*/)
{
	return NULL;
}

editor_action* mouse_action::drag_end(
		editor_display& /*disp*/, int /*x*/, int /*y*/)
{
	return NULL;
}

editor_action* mouse_action::up_right(
		editor_display& /*disp*/, int /*x*/, int /*y*/)
{
	return NULL;
}

editor_action* mouse_action::up_left(
		editor_display& /*disp*/, int /*x*/, int /*y*/)
{
	return NULL;
}

editor_action* mouse_action::key_event(
	editor_display& disp, const SDL_Event& event)
{
	if (!has_alt_modifier() && (event.key.keysym.sym >= '1' && event.key.keysym.sym <= '9')) {
		int side = event.key.keysym.sym - '0';
		if (side >= 1 && side <= gamemap::MAX_PLAYERS) {
			map_location pos = disp.get_map().starting_position(side);
			if (pos.valid()) {
				disp.scroll_to_tile(pos, display::WARP);
			}
		}
		return NULL;
	}
	if (!disp.map().on_board(previous_move_hex_) || event.type != SDL_KEYUP) {
		return NULL;
	}
	editor_action* a = NULL;
	if ((has_alt_modifier() && (event.key.keysym.sym >= '1' && event.key.keysym.sym <= '9'))
	|| event.key.keysym.sym == SDLK_DELETE) {
		int res = event.key.keysym.sym - '0';
		if (res > gamemap::MAX_PLAYERS || event.key.keysym.sym == SDLK_DELETE) res = 0;
		int player_starting_at_hex = disp.map().is_starting_position(previous_move_hex_) + 1;
		if (res == 0 && player_starting_at_hex != -1) {
			a = new editor_action_starting_position(map_location(), player_starting_at_hex);
		} else if (res > 0 && res != player_starting_at_hex) {
			a = new editor_action_starting_position(previous_move_hex_, res);
		}
	}
	return a;
}

void mouse_action::set_mouse_overlay(editor_display& disp)
{
	disp.set_mouseover_hex_overlay(NULL);
}

bool mouse_action::has_alt_modifier() const
{
	return key_[SDLK_RALT] || key_[SDLK_LALT];
}

bool mouse_action::has_shift_modifier() const
{
	return key_[SDLK_RSHIFT] || key_[SDLK_LSHIFT];
}

bool mouse_action::has_ctrl_modifier() const
{
#ifdef __APPLE__
	return key_[SDLK_RMETA] || key_[SDLK_LMETA];
#else
	return key_[SDLK_RCTRL] || key_[SDLK_LCTRL];
#endif
}

void mouse_action::set_terrain_mouse_overlay(editor_display& disp, t_translation::t_terrain fg,
		t_translation::t_terrain bg)
{
	surface image_fg(image::get_image("terrain/"
		+ disp.get_map().get_terrain_info(fg).editor_image() + ".png"));
	surface image_bg(image::get_image("terrain/"
		+ disp.get_map().get_terrain_info(bg).editor_image() + ".png"));

	if (image_fg == NULL || image_bg == NULL) {
		ERR_ED << "Missing terrain icon\n";
		disp.set_mouseover_hex_overlay(NULL);
		return;
	}

	// Create a transparent surface of the right size.
	surface image = create_compatible_surface(image_fg, image_fg->w, image_fg->h);
	SDL_FillRect(image,NULL,SDL_MapRGBA(image->format,0,0,0, 0));

	// For efficiency the size of the tile is cached.
	// We assume all tiles are of the same size.
	// The zoom factor can change, so it's not cached.
	// NOTE: when zooming and not moving the mouse, there are glitches.
	// Since the optimal alpha factor is unknown, it has to be calculated
	// on the fly, and caching the surfaces makes no sense yet.
	static const Uint8 alpha = 196;
	static const int size = image_fg->w;
	static const int half_size = size / 2;
	static const int quarter_size = size / 4;
	static const int offset = 2;
	static const int new_size = half_size - 2;
	const int zoom = static_cast<int>(size * disp.get_zoom_factor());

	// Blit left side
	image_fg = scale_surface(image_fg, new_size, new_size);
	SDL_Rect rcDestLeft = { offset, quarter_size, 0, 0 };
	SDL_BlitSurface ( image_fg, NULL, image, &rcDestLeft );

	// Blit left side
	image_bg = scale_surface(image_bg, new_size, new_size);
	SDL_Rect rcDestRight = { half_size, quarter_size, 0, 0 };
	SDL_BlitSurface ( image_bg, NULL, image, &rcDestRight );

	//apply mask so the overlay is contained within the mouseover hex
	surface mask(image::get_image("terrain/alphamask.png"));
	image = mask_surface(image, mask);

	// Add the alpha factor and scale the image
	image = scale_surface(adjust_surface_alpha(image, alpha), zoom, zoom);

	// Set as mouseover
	disp.set_mouseover_hex_overlay(image);
}

std::set<map_location> brush_drag_mouse_action::affected_hexes(
	editor_display& /*disp*/, const map_location& hex)
{
	return get_brush().project(hex);
}

editor_action* brush_drag_mouse_action::click_left(editor_display& disp, int x, int y)
{
	map_location hex = disp.hex_clicked_on(x, y);
	previous_drag_hex_ = hex;
	return click_perform_left(disp, affected_hexes(disp, hex));
}

editor_action* brush_drag_mouse_action::click_right(editor_display& disp, int x, int y)
{
	map_location hex = disp.hex_clicked_on(x, y);
	previous_drag_hex_ = hex;
	return click_perform_right(disp, affected_hexes(disp, hex));
}

editor_action* brush_drag_mouse_action::drag_left(editor_display& disp,
		int x, int y, bool& partial, editor_action* last_undo)
{
	return drag_generic<&brush_drag_mouse_action::click_perform_left>(disp, x, y, partial, last_undo);
}

editor_action* brush_drag_mouse_action::drag_right(editor_display& disp,
		int x, int y, bool& partial, editor_action* last_undo)
{
	return drag_generic<&brush_drag_mouse_action::click_perform_right>(disp, x, y, partial, last_undo);
}

editor_action* brush_drag_mouse_action::drag_end(
		editor_display& /*disp*/, int /*x*/, int /*y*/)
{
	return NULL;
}

template <editor_action* (brush_drag_mouse_action::*perform_func)(editor_display&, const std::set<map_location>&)>
editor_action* brush_drag_mouse_action::drag_generic(editor_display& disp, int x, int y, bool& partial, editor_action* last_undo)
{
	map_location hex = disp.hex_clicked_on(x, y);
	move(disp, hex);
	if (hex != previous_drag_hex_) {
		std::set<map_location> current_step_locs = affected_hexes(disp, hex);
		editor_action_extendable* last_undo_x = dynamic_cast<editor_action_extendable*>(last_undo);
		LOG_ED << "Last undo is " << last_undo << " and as x " << last_undo_x << "\n";
		partial = true;
		editor_action* a = (this->*perform_func)(disp, affected_hexes(disp, hex));
		previous_drag_hex_ = hex;
		return a;
	} else {
		return NULL;
	}
}

const brush& brush_drag_mouse_action::get_brush()
{
	assert(brush_);
	assert(*brush_);
	return **brush_;
}


editor_action* mouse_action_paint::click_left(editor_display& disp, int x, int y)
{
	if (has_ctrl_modifier()) {
		map_location hex = disp.hex_clicked_on(x, y);
		terrain_left_ = disp.map().get_terrain(hex);
		return NULL;
	} else {
		return brush_drag_mouse_action::click_left(disp, x, y);
	}
}

editor_action* mouse_action_paint::click_right(editor_display& disp, int x, int y)
{
	if (has_ctrl_modifier()) {
		map_location hex = disp.hex_clicked_on(x, y);
		terrain_right_ = disp.map().get_terrain(hex);
		return NULL;
	} else {
		return brush_drag_mouse_action::click_right(disp, x, y);
	}
}

editor_action* mouse_action_paint::click_perform_left(
		editor_display& /*disp*/, const std::set<map_location>& hexes)
{
	if (has_ctrl_modifier()) return NULL;
	return new editor_action_chain(new editor_action_paint_area(hexes, terrain_left_, has_shift_modifier()));
}

editor_action* mouse_action_paint::click_perform_right(
		editor_display& /*disp*/, const std::set<map_location>& hexes)
{
	if (has_ctrl_modifier()) return NULL;
	return new editor_action_chain(new editor_action_paint_area(hexes, terrain_right_, has_shift_modifier()));
}

void mouse_action_paint::set_mouse_overlay(editor_display& disp)
{
	set_terrain_mouse_overlay(disp, terrain_left_, terrain_right_);
}


std::set<map_location> mouse_action_select::affected_hexes(
	editor_display& disp, const map_location& hex)
{
	if (has_shift_modifier()) {
		return disp.map().get_contigious_terrain_tiles(hex);
	} else {
		return brush_drag_mouse_action::affected_hexes(disp, hex);
	}
}

editor_action* mouse_action_select::key_event(
		editor_display& disp, const SDL_Event& event)
{
	editor_action* ret = mouse_action::key_event(disp, event);
	update_brush_highlights(disp, previous_move_hex_);
	return ret;
}

editor_action* mouse_action_select::click_perform_left(
		editor_display& /*disp*/, const std::set<map_location>& hexes)
{
	return new editor_action_chain(new editor_action_select(hexes));
}

editor_action* mouse_action_select::click_perform_right(
		editor_display& /*disp*/, const std::set<map_location>& hexes)
{
	return new editor_action_chain(new editor_action_deselect(hexes));
}

void mouse_action_select::set_mouse_overlay(editor_display& disp)
{
	surface image;
	if (has_shift_modifier()) {
		image = image::get_image("editor/tool-overlay-select-wand.png");
	} else {
		image = image::get_image("editor/tool-overlay-select-brush.png");
	}
	Uint8 alpha = 196;
	int size = image->w;
	int zoom = static_cast<int>(size * disp.get_zoom_factor());

	// Add the alpha factor and scale the image
	image = scale_surface(adjust_surface_alpha(image, alpha), zoom, zoom);
	disp.set_mouseover_hex_overlay(image);
}


bool mouse_action_paste::has_context_menu() const
{
	return true;
}

std::set<map_location> mouse_action_paste::affected_hexes(
	editor_display& /*disp*/, const map_location& hex)
{
	return paste_.get_offset_area(hex);
}

editor_action* mouse_action_paste::click_left(editor_display& disp, int x, int y)
{
	map_location hex = disp.hex_clicked_on(x, y);
	editor_action_paste* a = new editor_action_paste(paste_, hex);
	return a;
}

editor_action* mouse_action_paste::click_right(editor_display& /*disp*/, int /*x*/, int /*y*/)
{
	return NULL;
}

void mouse_action_paste::set_mouse_overlay(editor_display& disp)
{
	disp.set_mouseover_hex_overlay(NULL); //TODO
}


std::set<map_location> mouse_action_fill::affected_hexes(
	editor_display& disp, const map_location& hex)
{
	return disp.map().get_contigious_terrain_tiles(hex);
}

editor_action* mouse_action_fill::click_left(editor_display& disp, int x, int y)
{
	map_location hex = disp.hex_clicked_on(x, y);
	if (has_ctrl_modifier()) {
		terrain_left_ = disp.map().get_terrain(hex);
		return NULL;
	} else {
		//TODO only take the base terrain into account when searching for contigious terrain when painting base only
		//or use a different key modifier for that
		editor_action_fill* a = new editor_action_fill(hex, terrain_left_, has_shift_modifier());
		return a;
	}
}

editor_action* mouse_action_fill::click_right(editor_display& disp, int x, int y)
{
	map_location hex = disp.hex_clicked_on(x, y);
	if (has_ctrl_modifier()) {
		map_location hex = disp.hex_clicked_on(x, y);
		terrain_right_ = disp.map().get_terrain(hex);
		return NULL;
	} else {
		//TODO only take the base terrain into account when searching for contigious terrain when painting base only
		//or use a different key modifier for that
		editor_action_fill* a = new editor_action_fill(hex, terrain_right_, has_shift_modifier());
		return a;
	}
}

void mouse_action_fill::set_mouse_overlay(editor_display& disp)
{
	set_terrain_mouse_overlay(disp, terrain_left_, terrain_right_);
}

editor_action* mouse_action_village::click_right(editor_display& /*disp*/, int /*x*/, int /*y*/)
{
	return NULL;
}

editor_action* mouse_action_village::click_left(editor_display& /*disp*/, int /*x*/, int /*y*/)
{
	return NULL;
}

editor_action* mouse_action_village::up_left(editor_display& disp, int x, int y)
{
	map_location hex = disp.hex_clicked_on(x, y);
	return new editor_action_village(hex, disp.playing_team());
}

editor_action* mouse_action_village::up_right(editor_display& disp, int x, int y)
{
	map_location hex = disp.hex_clicked_on(x, y);
	return new editor_action_village_delete(hex);
}



editor_action* mouse_action_unit::click_right(editor_display& disp, int x, int y)
{
	map_location hex = disp.hex_clicked_on(x, y);
	const unit_map& units = disp.map().get_const_units();

	last_hex_ = hex;

	const unit_map::const_unit_iterator unit_it = units.find(hex);
	if (unit_it != units.end()) {
//		surface image = unit_it->second.still_image();
//		Uint8 alpha = 196;
//		int size = image->w;
//		int zoom = static_cast<int>(size * disp.get_zoom_factor());
//		// Add the alpha factor and scale the image
//		image = scale_surface(adjust_surface_alpha(image, alpha), zoom, zoom);
//		disp.set_mouseover_hex_overlay(image);
		temp_unit_ = &*unit_it;

		//TODO set the mouse pointer to a draging one.

	} else { temp_unit_ = NULL; }

	click_ = true;
	return NULL;
}
editor_action* mouse_action_unit::drag_right(editor_display& disp, int x, int y, bool& partial, editor_action* last_undo)
{
	map_location hex = disp.hex_clicked_on(x, y);
	if (last_hex_ != hex) {
		last_hex_ = hex;
		if (temp_unit_) {
			for (map_location::DIRECTION direction = map_location::NORTH;
					direction <= map_location::NORTH_WEST;
					direction = map_location::DIRECTION(direction +1)){
				if (temp_unit_->get_location().get_direction(direction, 1) == hex) {
					unit& u = *disp.map().get_units().find(temp_unit_->get_location());
					u.set_facing(direction);
					u.set_standing();
				}
			}
		}
	}
	return NULL;
}
editor_action* mouse_action_unit::up_right(editor_display& disp, int x, int y)
{
	map_location hex = disp.hex_clicked_on(x, y);
	return new editor_action_unit_delete(hex);
}

editor_action* mouse_action_unit::click_left(editor_display& disp, int x, int y)
{
	map_location hex = disp.hex_clicked_on(x, y);
	const unit_map& units = disp.map().get_const_units();

	const unit_map::const_unit_iterator unit_it = units.find(hex);
	if (unit_it != units.end()) {
		surface image = unit_it->still_image();
		Uint8 alpha = 196;
		int size = image->w;
		int zoom = static_cast<int>(size * disp.get_zoom_factor());
		// Add the alpha factor and scale the image
		image = scale_surface(adjust_surface_alpha(image, alpha), zoom, zoom);
		disp.set_mouseover_hex_overlay(image);
		temp_unit_ = &*unit_it;

		//TODO set the mouse pointer to a draging one.

	} else { temp_unit_ = NULL; }

	click_ = true;
	return NULL;
}

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
	return new editor_action_label_delete(hex);
}
editor_action* mouse_action_map_label::drag_left(editor_display& disp, int x, int y
		, bool& partial, editor_action* last_undo)
{
	map_location hex = disp.hex_clicked_on(x, y);

	if (hex == last_draged_)
		return NULL;

	//TODO this is somewhat hacky.
	disp.labels().clear_all();
	//TODO How can they be redrawn?

	last_draged_ = hex;

	const label* clicked_label = disp.map().get_game_labels().get_label(clicked_on_);
	if (clicked_label) {
		std::string text = clicked_label->text() + "\n";
		const label* hex_label = disp.labels().get_label(hex);
		//TODO the stacking is not working because we don't redraw all the labels.
		if (hex_label)
			text += hex_label->text();

		terrain_label* onscreen = new terrain_label(text, "", hex, disp.labels(),
				font::LABEL_COLOUR, true, true, false);
		disp.labels().add_label(hex, onscreen);
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
		const label& label_clicked = *(disp.map().get_game_labels().get_label(clicked_on_));

		chain = new editor_action_chain();
		chain->append_action(new editor_action_label(hex, label_clicked.text(), label_clicked.team_name()));
		chain->append_action(new editor_action_label_delete(clicked_on_));
	}
	return chain;
}



editor_action* mouse_action_map_label::up_left(editor_display& disp, int x, int y)
{
	if (!click_) return NULL;
	click_ = false;
	map_location hex = disp.hex_clicked_on(x, y);
	if (!disp.map().on_board(hex)) {
		return NULL;
	}

	const label* old_label = disp.map().get_game_labels().get_label(hex);

	bool visible_in_fog = true;
	bool visible_in_shroud = true;

	//TODO gui2
	gui::dialog	d(disp, _("Place Label"), "", gui::OK_CANCEL);
	d.set_textbox(_("Label: "), (old_label ? old_label->text() : ""), map_labels::get_max_chars());
	//TODO note that gui1 does not support more than one textbox in a dialogue.
	//	d.set_textbox(_("Team: "), (old_label ? old_label->team_name() : ""), map_labels::get_max_chars());
	d.add_option(_("Visible in Fog"), visible_in_fog, gui::dialog::BUTTON_CHECKBOX_LEFT);
	d.add_option(_("Visible in Shroud"), visible_in_shroud, gui::dialog::BUTTON_CHECKBOX_LEFT);
	//color can be adjusted as well
	//	d.add_option(_("Team only"), team_only, gui::dialog::BUTTON_CHECKBOX_LEFT);
	//TODO
	const std::string team_name = "";

	editor_action* a = NULL;
	if(!d.show()) {
		a = new editor_action_label(hex, d.textbox_text(), team_name);
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

void mouse_action_village::set_mouse_overlay(editor_display& disp)
{
	surface image = image::get_image("editor/tool-overlay-village.png");
	Uint8 alpha = 196;
	int size = image->w;
	int zoom = static_cast<int>(size * disp.get_zoom_factor());

	// Add the alpha factor and scale the image
	image = scale_surface(adjust_surface_alpha(image, alpha), zoom, zoom);
	disp.set_mouseover_hex_overlay(image);
}

void mouse_action_unit::set_mouse_overlay(editor_display& disp)
{
	//TODO change to a new image.
	surface image = image::get_image("editor/tool-overlay-starting-position.png");
	Uint8 alpha = 196;
	int size = image->w;
	int zoom = static_cast<int>(size * disp.get_zoom_factor());

	// Add the alpha factor and scale the image
	image = scale_surface(adjust_surface_alpha(image, alpha), zoom, zoom);
	disp.set_mouseover_hex_overlay(image);
}


editor_action* mouse_action_starting_position::up_left(editor_display& disp, int x, int y)
{
	if (!click_) return NULL;
	click_ = false;
	map_location hex = disp.hex_clicked_on(x, y);
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
				       _("Which player should start here? You can use alt and a number key to set the starting position for a player, and del to clear the starting position under the cursor. Pressing a number key by itself will scroll to that player's starting position."),
				       gui::OK_CANCEL);
	pmenu.set_menu(players);
	int res = pmenu.show();
	editor_action* a = NULL;
	if (res == 0 && player_starting_at_hex != -1) {
		a = new editor_action_starting_position(map_location(), player_starting_at_hex);
	} else if (res > 0 && res != player_starting_at_hex) {
		a = new editor_action_starting_position(hex, res);
	}
	update_brush_highlights(disp, hex);
	return a;
}


editor_action* mouse_action_unit::drag_end(editor_display& disp, int x, int y) {

	if (!click_) return NULL;
	click_ = false;

	editor_action* a = NULL;

	if(temp_unit_) {
		map_location hex = disp.hex_clicked_on(x, y);
		if (!disp.map().on_board(hex))
			return NULL;
		unit u = *temp_unit_;
		a = new editor_action_unit_replace(u.get_location(), hex);
	}
//	ERR_ED << "\n Drag end at " << hex.x << "/" << hex.y;
	return a;
}




editor_action* mouse_action_unit::up_left(editor_display& disp, int x, int y)
{
	if (!click_) return NULL;
	click_ = false;
	map_location hex = disp.hex_clicked_on(x, y);
	if (!disp.map().on_board(hex)) {
		return NULL;
	}

	if (temp_unit_) {
		config u;
		temp_unit_->write(u);

		// TODO: see the comments below
		//vconfig cfg;
		//gui2::tconfig_inspector inspect_dialog(cfg, u);
		//inspect_dialog.show(disp.video());

	}

	//TODO the gui1 dialog works better.
	//
	// The unit creation dialog makes sure unit types
	// are properly cached.
	//
	gui2::tunit_create create_dlg;
	create_dlg.show(disp.video());

	if(create_dlg.no_choice()) {
		return NULL;
	}

	const std::string& ut_id = create_dlg.choice();
	const unit_type *utp = unit_types.find(ut_id);
	if (!utp) {
		ERR_ED << "create unit dialog returned inexistent or unusable unit_type id '" << ut_id << "'\n";
		return NULL;
	}

	const unit_type &ut = *utp;

	unit_race::GENDER gender = create_dlg.gender();
	//TODO
	//			const bool generate_name = create_dlg.generate_name();

	// Do not try to set bad genders, may mess up l10n
	// FIXME: is this actually necessary?
	if(ut.genders().end() == std::find(ut.genders().begin(), ut.genders().end(), gender)) {
		gender = ut.genders().front();
	}

	bool canrecruit = disp.map().get_teams()[disp.get_playing_team() ].no_leader();

	if (canrecruit) disp.map().get_teams()[disp.get_playing_team() ].have_leader(true);
	// FIXME: This may NOT work as intended, as the unit_map to add the unit to cannot be specified.
	// Blame silene for removing that argument from unit's constructors
	//unit u(&disp.map().get_units(), utp, disp.get_playing_team() +1, false, gender, canrecruit);
	unit u(utp, disp.get_playing_team() +1, false, gender, canrecruit);


	editor_action* a = new editor_action_unit(hex, u);
	return a;
}


editor_action* mouse_action_starting_position::click_left(editor_display& /*disp*/, int /*x*/, int /*y*/)
{
	click_ = true;
	return NULL;
}

editor_action* mouse_action_starting_position::up_right(editor_display& disp, int x, int y)
{
	//TODO
//	map_location hex = disp.hex_clicked_on(x, y);
//	int player_starting_at_hex = disp.map().is_starting_position(hex) + 1;
//	if (player_starting_at_hex != -1) {
//		config side;
////		disp.get_teams()[player_starting_at_hex].write(side);
//		disp.get_teams()[1].write(side);
//		//const config const_side = side;
//		vconfig vside(side, true);
//		ERR_ED << vside.get_config().debug();
//		gui2::tgamestate_inspector inspect_dialog(vside);
//		inspect_dialog.show(disp.video());
////		inspect_dialog.show(resources::screen->video());
//		return NULL;
//		//return new editor_action_side_config(map_location(), player_starting_at_hex);
//	} else {
		return NULL;
//	}
}

editor_action* mouse_action_starting_position::click_right(editor_display& /*disp*/, int /*x*/, int /*y*/)
{
	return NULL;
}

void mouse_action_starting_position::set_mouse_overlay(editor_display& disp)
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
