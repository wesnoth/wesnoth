/*
  Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
  Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License.
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY.

  See the COPYING file for more details.
*/

#include "SDL.h"
#include "SDL_keysym.h"

#include "../config.hpp"
#include "../cursor.hpp"
#include "../dialogs.hpp"
#include "../filesystem.hpp"
#include "../font.hpp"
#include "../game_config.hpp"
#include "../gamestatus.hpp"
#include "../key.hpp"
#include "../widgets/menu.hpp"
#include "../language.hpp"
#include "../playlevel.hpp"
#include "../preferences.hpp"
#include "../sdl_utils.hpp"
#include "../tooltips.hpp"
#include "../team.hpp"
#include "../unit_types.hpp"
#include "../unit.hpp"
#include "../util.hpp"
#include "../video.hpp"

#include "editor.hpp"
#include "map_manip.hpp"
#include "editor_dialogs.hpp"
#include "editor_palettes.hpp"

#include <cctype>
#include <iostream>
#include <map>
#include <string>
#include <cmath>

namespace {
	// Milliseconds to sleep in every iteration of the main loop.
	const unsigned int sdl_delay = 20;
	const std::string prefs_filename = get_dir(get_user_data_dir() + "/editor")
		+ "/preferences";
	

	void terrain_changed(gamemap &map, const gamemap::location &hex) {
		// If we painted something else than a keep on a starting position,
		// unset the starting position.
		int start_side = -1;
		for (int i = 0; i < 10; i++) {
			if (map.starting_position(i) == hex) {
				start_side = i;
			}
		}
		if (start_side != -1 && map.get_terrain(hex) != gamemap::KEEP) {
			map.set_starting_position(start_side, gamemap::location());
		}
	}

	double location_distance(const gamemap::location loc1, const gamemap::location loc2) {
		const double xdiff = loc1.x - loc2.x;
		const double ydiff = loc1.y - loc2.y;
		const double dist = sqrt(xdiff * xdiff + ydiff * ydiff);
		return dist;
	}

}

namespace map_editor {
map_editor::map_editor(display &gui, gamemap &map, config &theme, config &game_config)
	: gui_(gui), map_(map), abort_(DONT_ABORT),
	  num_operations_since_save_(0), theme_(theme), game_config_(game_config),
	  map_dirty_(false), palette_(gui, size_specs_, map), brush_(gui, size_specs_),
	  l_button_func_(NONE), prefs_disp_manager_(&gui_) {

	// Set size specs.
	adjust_sizes(gui_, size_specs_);
	palette_.adjust_size();
	brush_.adjust_size();

	// Clear the current hotkeys. Alot of hotkeys are already set
	// through other configuration files (e.g. english.cfg and
	// preferences) and we need to clear these or they will overlap.
	hotkey::get_hotkeys().clear();
	hotkey::add_hotkeys(theme_, true);
	try {
		prefs_.read(read_file(prefs_filename));
	}
	catch (config::error e) {
		std::cerr << "Error when reading " << prefs_filename << ": "
				  << e.message << std::endl;
	}
	hotkey::add_hotkeys(prefs_, true);

	recalculate_starting_pos_labels();

	gui_.begin_game();
	gui_.invalidate_all();
	gui_.draw();
	events::raise_draw_event();
}

map_editor::~map_editor() {
	try {
		write_file(prefs_filename, prefs_.write());
	}
	catch (io_exception& e) {
		std::cerr << "Error when writing to " << prefs_filename << ": "
				  << e.what() << std::endl;
	}
}

void map_editor::handle_event(const SDL_Event &event) {
	int mousex, mousey;
	SDL_GetMouseState(&mousex,&mousey);
	const SDL_KeyboardEvent keyboard_event = event.key;
	handle_keyboard_event(keyboard_event, mousex, mousey);

	const SDL_MouseButtonEvent mouse_button_event = event.button;
	handle_mouse_button_event(mouse_button_event, mousex, mousey);
}

void map_editor::handle_keyboard_event(const SDL_KeyboardEvent &event,
									   const int /*mousex*/, const int /*mousey*/) {
	if (event.type == SDL_KEYDOWN) {
		const SDLKey sym = event.keysym.sym;
		// We must intercept escape-presses here because we don't want the
		// default shutdown behavior, we want to ask for saving.
		if (sym == SDLK_ESCAPE) {
			set_abort();
		}
		else {
			const bool old_fullscreen = preferences::fullscreen();
			const std::pair<int, int> old_resolution = preferences::resolution();
			hotkey::key_event(gui_, event, this);
			// A key event may change the video mode. The redraw
			// functionality inside the preferences module does not
			// redraw our palettes so we need to check if the mode has
			// changed and if so redraw everything.
			if (preferences::fullscreen() != old_fullscreen
				|| old_resolution != preferences::resolution()) {
				redraw_everything();
			}
		}
	}
}

void map_editor::handle_mouse_button_event(const SDL_MouseButtonEvent &event,
										   const int mousex, const int mousey) {
	if (event.type == SDL_MOUSEBUTTONDOWN) {
		const Uint8 button = event.button;
		if (button == SDL_BUTTON_RIGHT) {
			selected_hex_ = gui_.hex_clicked_on(mousex, mousey);
			const theme::menu* const m = gui_.get_theme().context_menu();
			show_menu(m->items(), mousex, mousey + 1, true);
		}
		if (button == SDL_BUTTON_LEFT) {
			gamemap::location hex_clicked = gui_.hex_clicked_on(mousex, mousey);
			if (map_.on_board(hex_clicked)) {
				if (key_[SDLK_LSHIFT] || key_[SDLK_RSHIFT]) {
					if (selected_hexes_.find(hex_clicked)
						== selected_hexes_.end()) {
						l_button_func_ = ADD_SELECTION;
					}
					else {
						l_button_func_ = REMOVE_SELECTION;
					}
				}
				else if (selected_hexes_.find(hex_clicked) != selected_hexes_.end()) {
					l_button_func_ = MOVE_SELECTION;
					selection_move_start_ = hex_clicked;
				}
				else if (!selected_hexes_.empty()) {
					// If hexes are selected, clear them and do not draw
					// anything.
					selected_hexes_.clear();
					gui_.clear_highlighted_locs();
					update_mouse_over_hexes(hex_clicked);
				}
				else {
					l_button_func_ = DRAW_TERRAIN;
				}
			}
			else {
			}
		}
	}
	if (event.type == SDL_MOUSEBUTTONUP) {
		// If we miss the mouse up event we need to perform the actual
		// movement if we are in the progress of moving a selection.
		if (l_button_func_ == MOVE_SELECTION) {
			perform_selection_move();
		}
		l_button_func_ = NONE;
	}
}


void map_editor::edit_save_as() {
	std::string input_name = get_dir(get_dir(get_user_data_dir() + "/editor") + "/maps/");
	int res = 0;
	int overwrite = 0;
	do {
		res = gui::show_dialog(gui_, NULL, "", "Save As ", gui::OK_CANCEL,
							   NULL, NULL, "", &input_name);
					   
		if (res == 0) {
			if (file_exists(input_name)) {
				overwrite = gui::show_dialog(gui_, NULL, "Overwrite?",
											 "Map already exists. Do you want to overwrite it?",
											 gui::YES_NO);
			}
			else
				overwrite = 0;
		}
	} while (res == 0 && overwrite != 0);

	if (res == 0) {
		set_file_to_save_as(input_name);
		save_map("", true);
	}
}

void map_editor::edit_set_start_pos() {
	std::string player = "1";
	const int res = gui::show_dialog(gui_, NULL, "Which Player?",
									 "Which player should start here?",
									 gui::OK_CANCEL, NULL, NULL, "", &player);
	if (player != "" && res == 0) {
		int int_player;
		bool invalid_number = player.size() > 1;
		std::stringstream str(player);
		str >> int_player;
		invalid_number = invalid_number ? true : int_player < 0 || int_player > 9;
		if (invalid_number) { 
			gui::show_dialog(gui_, NULL, "",
							 "You must enter a number between 0 and 9.", gui::OK_ONLY);
		}
		else {
			set_starting_position(int_player, selected_hex_);
		}
	}
}

void map_editor::edit_flood_fill() {
	terrain_log log;
	flood_fill(map_, selected_hex_, palette_.selected_terrain(), &log);
	std::vector<gamemap::location> to_invalidate;
	map_undo_action action;
	for (terrain_log::iterator it = log.begin(); it != log.end(); it++) {
		to_invalidate.push_back((*it).first);
		action.add_terrain((*it).second, palette_.selected_terrain(),
			   (*it).first);
	}
	save_undo_action(action);
	invalidate_all_and_adjacent(to_invalidate);
}

void map_editor::edit_save_map() {
	save_map("", true);
}

void map_editor::edit_quit() {
	set_abort();
}

void map_editor::edit_new_map() {
	const std::string map = new_map_dialog(gui_, palette_.selected_terrain(),
										   changed_since_save(), game_config_);
 	if (map != "") {
		clear_undo_actions();
		throw new_map_exception(map);
	}
}

void map_editor::edit_load_map() {
	const std::string fn = load_map_dialog(gui_);
	if (fn != "") {
		const std::string new_map = load_map(fn);
		if (new_map != "") {
			if (!changed_since_save() || confirm_modification_disposal(gui_)) {
				clear_undo_actions();
				throw new_map_exception(new_map, fn);
			}
		}
	}
}

void map_editor::edit_fill_selection() {
	map_undo_action undo_action;
	for (std::set<gamemap::location>::const_iterator it = selected_hexes_.begin();
		 it != selected_hexes_.end(); it++) {
		if (map_.on_board(*it)) {
			undo_action.add_terrain(map_.get_terrain(*it), palette_.selected_terrain(), *it);
			map_.set_terrain(*it, palette_.selected_terrain());
		}
	}
	save_undo_action(undo_action);
	invalidate_all_and_adjacent(selected_hexes_);
}

void map_editor::edit_cut() {
	clipboard_.clear();
	insert_selection_in_clipboard();
	edit_fill_selection();
}

void map_editor::edit_copy() {
	clipboard_.clear();
	insert_selection_in_clipboard();
}

void map_editor::edit_paste() {
	map_undo_action undo_action;
	std::set<gamemap::location> filled;
	gamemap::location start_hex = selected_hex_;
	gui_.clear_highlighted_locs();
	for (std::vector<clipboard_item>::const_iterator it = clipboard_.begin();
		 it != clipboard_.end(); it++) {
		gamemap::location l(clipboard_offset_loc_.x + (*it).x_offset,
							clipboard_offset_loc_.y + (*it).y_offset);
		const int x_offset = start_hex.x - clipboard_offset_loc_.x;
		const int y_offset = start_hex.y - clipboard_offset_loc_.y;
		gamemap::location target = get_hex_with_offset(l, x_offset, y_offset);
		if (map_.on_board(target)) {
			undo_action.add_terrain(map_.get_terrain(target), (*it).terrain, target);
			map_.set_terrain(target, (*it).terrain);
			filled.insert(target);
			gui_.add_highlighted_loc(target);
		}
	}
	undo_action.set_selection(filled, selected_hexes_);
	invalidate_all_and_adjacent(filled);
	selected_hexes_ = filled;
	save_undo_action(undo_action, false);
}

void map_editor::edit_revert() {
	const std::string new_map = load_map(filename_);
	if (new_map != "") {
		map_undo_action action;
		action.set_type(map_undo_action::WHOLE_MAP);
		action.set_map_data(map_.write(), new_map);
		save_undo_action(action, false);
		throw new_map_exception(new_map, filename_);
	}
}

void map_editor::edit_resize() {
	const std::pair<unsigned, unsigned> new_size =
		resize_dialog(gui_, map_.x(), map_.y());
	if (new_size.first != 0) {
		const std::string resized_map =
			resize_map(map_, new_size.first, new_size.second, palette_.selected_terrain());
		if (resized_map != "") {
			map_undo_action action;
			action.set_type(map_undo_action::WHOLE_MAP);
			action.set_map_data(map_.write(), resized_map);
			save_undo_action(action, false);
			throw new_map_exception(resized_map, filename_);
		}
	}
}

std::string map_editor::load_map(const std::string filename) {
	bool load_successful = true;
	std::string msg = "'";
	std::string new_map;
	if (!file_exists(filename) || is_directory(filename)) {
		load_successful = false;
		msg += filename + "' does not exist or can't be read as a file.";
	}
	else {
		try {
			new_map = read_file(filename);
		}
		catch (io_exception e) {
			load_successful = false;
			msg = e.what();
		}
	}
	if (!load_successful) {
		gui::show_dialog(gui_, NULL, "", std::string("Load failed: ") + msg, gui::OK_ONLY);
		return "";
	}
	else {
		return new_map;
	}
}


void map_editor::insert_selection_in_clipboard() {
	if (selected_hexes_.empty()) {
		return;
	}
	gamemap::location offset_hex = *(selected_hexes_.begin());
	std::set<gamemap::location>::const_iterator it;
	// Find the hex that is closest to the selected one, use this as the
	// one to calculate the offset from.
	for (it = selected_hexes_.begin(); it != selected_hexes_.end(); it++) {
		if (location_distance(selected_hex_, *it) <
			location_distance(selected_hex_, offset_hex)) {
			offset_hex = *it;
		}
	}
	clipboard_offset_loc_ = offset_hex;
	for (it = selected_hexes_.begin(); it != selected_hexes_.end(); it++) {
		const int x_offset = (*it).x - offset_hex.x;
		const int y_offset = (*it).y - offset_hex.y;
		gamemap::TERRAIN terrain = map_.get_terrain(*it);
		clipboard_.push_back(clipboard_item(x_offset, y_offset, terrain));
	}
}


bool map_editor::can_execute_command(hotkey::HOTKEY_COMMAND command) const {
	switch (command) {
	case hotkey::HOTKEY_UNDO:
	case hotkey::HOTKEY_REDO:
	case hotkey::HOTKEY_ZOOM_IN:
	case hotkey::HOTKEY_ZOOM_OUT:
	case hotkey::HOTKEY_ZOOM_DEFAULT:
	case hotkey::HOTKEY_FULLSCREEN:
	case hotkey::HOTKEY_TOGGLE_GRID:
	case hotkey::HOTKEY_PREFERENCES:
	case hotkey::HOTKEY_EDIT_SAVE_MAP:
	case hotkey::HOTKEY_EDIT_SAVE_AS:
	case hotkey::HOTKEY_EDIT_QUIT:
	case hotkey::HOTKEY_EDIT_SET_START_POS:
	case hotkey::HOTKEY_EDIT_NEW_MAP:
	case hotkey::HOTKEY_EDIT_LOAD_MAP:
	case hotkey::HOTKEY_EDIT_FLOOD_FILL:
	case hotkey::HOTKEY_EDIT_FILL_SELECTION:
	case hotkey::HOTKEY_EDIT_COPY:
	case hotkey::HOTKEY_EDIT_CUT:
	case hotkey::HOTKEY_EDIT_PASTE:
	case hotkey::HOTKEY_EDIT_REVERT:
	case hotkey::HOTKEY_EDIT_RESIZE:
		return true;
	default:
		return false;
	}
}

void map_editor::toggle_grid() {
	preferences::set_grid(!preferences::grid());
	gui_.set_grid(preferences::grid());
	gui_.invalidate_all();
}

void map_editor::save_undo_action(map_undo_action &action, const bool keep_selection) {
	if (keep_selection) {
		action.set_selection(selected_hexes_, selected_hexes_);
	}
	add_undo_action(action);
	num_operations_since_save_++;
}

void map_editor::undo() {
	if(exist_undo_actions()) {
		--num_operations_since_save_;
		map_undo_action action = pop_undo_action();
		if (action.undo_type() == map_undo_action::REGULAR) {
			selected_hexes_ = action.undo_selection();
			highlight_selected_hexes(true);
			std::vector<gamemap::location> to_invalidate;
			for(std::map<gamemap::location,gamemap::TERRAIN>::const_iterator it =
					action.undo_terrains().begin();
				it != action.undo_terrains().end(); ++it) {
				map_.set_terrain(it->first, it->second);
				to_invalidate.push_back(it->first);
			}
			std::copy(selected_hexes_.begin(), selected_hexes_.end(),
					  std::back_inserter(to_invalidate));
			invalidate_all_and_adjacent(to_invalidate);
		}
		else if (action.undo_type() == map_undo_action::WHOLE_MAP) {
			throw new_map_exception(action.old_map_data(), filename_);
		}
	}
}

void map_editor::redo() {
	if(exist_redo_actions()) {
		++num_operations_since_save_;
		map_undo_action action = pop_redo_action();
		selected_hexes_ = action.redo_selection();
		if (action.undo_type() == map_undo_action::REGULAR) {
			highlight_selected_hexes(true);
			std::vector<gamemap::location> to_invalidate;
			for(std::map<gamemap::location,gamemap::TERRAIN>::const_iterator it =
					action.redo_terrains().begin();
				it != action.redo_terrains().end(); ++it) {
				map_.set_terrain(it->first, it->second);
				to_invalidate.push_back(it->first);
			}
			std::copy(selected_hexes_.begin(), selected_hexes_.end(),
					  std::back_inserter(to_invalidate));
			invalidate_all_and_adjacent(to_invalidate);
		}
		else if (action.undo_type() == map_undo_action::WHOLE_MAP) {
			throw new_map_exception(action.new_map_data(), filename_);
		}
	}
}

void map_editor::preferences() {
	preferences_dialog(gui_, prefs_);
	redraw_everything();
}

void map_editor::redraw_everything() {
	adjust_sizes(gui_, size_specs_);
	palette_.adjust_size();
	brush_.adjust_size();
	gui_.redraw_everything();
	palette_.draw(true);
	brush_.draw(true);
}

void map_editor::highlight_selected_hexes(const bool clear_old) {
	if (clear_old) {
		gui_.clear_highlighted_locs();
	}
	for (std::set<gamemap::location>::const_iterator it = selected_hexes_.begin();
		 it != selected_hexes_.end(); it++) {
		gui_.add_highlighted_loc(*it);
	}
}

bool map_editor::changed_since_save() const {
	return num_operations_since_save_ != 0;
}

void map_editor::set_starting_position(const int player, const gamemap::location loc) {
	if(map_.on_board(loc)) {
		map_.set_terrain(selected_hex_, gamemap::KEEP);
		// This operation is currently not undoable, so we need to make sure
		// that save is always asked for after it is performed.
		num_operations_since_save_++;
		map_.set_starting_position(player, loc);
		invalidate_adjacent(loc);
	}
	else {
		gui::show_dialog(gui_, NULL, "",
						 "You must have a hex selected on the board.", gui::OK_ONLY);
	}
}

void map_editor::set_abort(const ABORT_MODE abort) {
	abort_ = abort;
}

void map_editor::set_file_to_save_as(const std::string filename) {
	filename_ = filename;
}

gamemap::location map_editor::get_hex_with_offset(const gamemap::location loc,
												  const int x_offset, const int y_offset) {
	gamemap::location new_loc(loc.x + x_offset, loc.y + y_offset);
	if (new_loc.x % 2 != loc.x % 2 && is_odd(new_loc.x)) {
		new_loc.y--;
	}
	return new_loc;
}

void map_editor::left_button_down(const int mousex, const int mousey) {
	const gamemap::location& minimap_loc = gui_.minimap_location_on(mousex,mousey);
	const gamemap::location hex = gui_.hex_clicked_on(mousex, mousey);
	if (minimap_loc.valid()) {
		gui_.scroll_to_tile(minimap_loc.x,minimap_loc.y,display::WARP,false);
	}
	else if (key_[SDLK_RSHIFT] || key_[SDLK_LSHIFT]) {
		if (key_[SDLK_RALT] || key_[SDLK_LALT]) {
			// Select/deselect a component.
			std::set<gamemap::location> selected;
			selected = get_component(map_, selected_hex_);
			for (std::set<gamemap::location>::const_iterator it = selected.begin();
				 it != selected.end(); it++) {
				if (l_button_func_ == ADD_SELECTION) {
					gui_.add_highlighted_loc(*it);
					selected_hexes_.insert(*it);
				}
				else {
					gui_.remove_highlighted_loc(*it);
					selected_hexes_.erase(*it);
				}
			}
			update_mouse_over_hexes(hex);
		}
		else {
			// Select what the brush is over.
			std::vector<gamemap::location> selected;
			selected = get_tiles(map_, hex, brush_.selected_brush_size());
			for (std::vector<gamemap::location>::const_iterator it = selected.begin();
				 it != selected.end(); it++) {
				if (l_button_func_ == ADD_SELECTION) {
					gui_.add_highlighted_loc(*it);
					selected_hexes_.insert(*it);
				}
				else {
					gui_.remove_highlighted_loc(*it);
					selected_hexes_.erase(*it);
				}
			}
			update_mouse_over_hexes(hex);
		}
	}
	// If the left mouse button is down and we beforhand have registred
	// a mouse down event, draw terrain at the current location.
	else if (l_button_func_ == DRAW_TERRAIN) {
		if(map_.on_board(hex)) {
			const gamemap::TERRAIN terrain = map_[hex.x][hex.y];
			if(key_[SDLK_RCTRL] || key_[SDLK_LCTRL]) {
				if(palette_.selected_terrain() != terrain) {
					palette_.select_terrain(terrain);
				}
			}
			else {
				// optimize for common case
				if(brush_.selected_brush_size() == 1) {
					if(palette_.selected_terrain() != terrain) {
						draw_terrain(palette_.selected_terrain(), hex);
					}
				}
				else {
					std::vector<gamemap::location> locs =
						get_tiles(map_, hex, brush_.selected_brush_size());
					map_undo_action action;
					std::vector<gamemap::location> to_invalidate;
					for(std::vector<gamemap::location>::const_iterator it = locs.begin();
					    it != locs.end(); ++it) {
						if(palette_.selected_terrain() != map_[it->x][it->y]) {
							to_invalidate.push_back(*it);
							action.add_terrain(map_[it->x][it->y], palette_.selected_terrain(), *it);
							map_.set_terrain(*it, palette_.selected_terrain());
						}
					}
					if (!to_invalidate.empty()) {
						save_undo_action(action);
						invalidate_all_and_adjacent(to_invalidate);
					}
				}
			}
		}
	}
	else if (l_button_func_ == MOVE_SELECTION) {
		const int x_diff = hex.x - selection_move_start_.x;
		const int y_diff = hex.y - selection_move_start_.y;
		// No other selections should be active when doing this.
		gui_.clear_highlighted_locs();
		for (std::set<gamemap::location>::const_iterator it = selected_hexes_.begin();
			 it != selected_hexes_.end(); it++) {
			const gamemap::location hl_loc =
				get_hex_with_offset(*it, x_diff, y_diff);
			if (map_.on_board(hl_loc)) {
				gui_.add_highlighted_loc(hl_loc);
			}
		}
	}
}

void map_editor::perform_selection_move() {
	map_undo_action undo_action;
	const int x_diff = selected_hex_.x - selection_move_start_.x;
	const int y_diff = selected_hex_.y - selection_move_start_.y;
	gui_.clear_highlighted_locs();
	std::set<gamemap::location> new_selection;
	// Transfer the terrain to the new position.
	std::set<gamemap::location>::const_iterator it;
	for(it = selected_hexes_.begin(); it != selected_hexes_.end(); it++) {
		const gamemap::location hl_loc =
			get_hex_with_offset(*it, x_diff, y_diff);
		if (map_.on_board(hl_loc)) {
			undo_action.add_terrain(map_.get_terrain(hl_loc), map_.get_terrain(*it), hl_loc);
			gui_.add_highlighted_loc(hl_loc);
			map_.set_terrain(hl_loc, map_.get_terrain(*it));
			new_selection.insert(hl_loc);
		}
	}

	// Fill the selection with the selected terrain.
	for (it = selected_hexes_.begin(); it != selected_hexes_.end(); it++) {
		if (map_.on_board(*it) && new_selection.find(*it) == new_selection.end()) {
			undo_action.add_terrain(map_.get_terrain(*it), palette_.selected_terrain(), *it);
			map_.set_terrain(*it, palette_.selected_terrain());
		}
	}
	undo_action.set_selection(new_selection, selected_hexes_);
	invalidate_all_and_adjacent(selected_hexes_);
	selected_hexes_ = new_selection;
	invalidate_all_and_adjacent(selected_hexes_);
	save_undo_action(undo_action, false);
}

void map_editor::draw_terrain(const gamemap::TERRAIN terrain,
							  const gamemap::location hex) {
	const gamemap::TERRAIN current_terrain = map_[hex.x][hex.y];
	map_undo_action undo_action(current_terrain, terrain, hex);
	save_undo_action(undo_action);
	map_.set_terrain(hex, terrain);
	invalidate_adjacent(hex);
}

void map_editor::invalidate_adjacent(const gamemap::location hex) {
	terrain_changed(map_, hex);
	gamemap::location locs[7];
	locs[0] = hex;
	get_adjacent_tiles(hex,locs+1);
	for(int i = 0; i != 7; ++i) {
		if (!map_.on_board(locs[i])) {
			// One adjacent square is not on the board, clear it from
			// the border cache so it will be redrawn correctly.
			gamemap::TERRAIN terrain_before = map_.get_terrain(locs[i]);
			map_.remove_from_border_cache(locs[i]);
			gamemap::TERRAIN terrain_after = map_.get_terrain(locs[i]);
			if (terrain_before != terrain_after) {
				invalidate_adjacent(locs[i]);
			}
		}
		gui_.rebuild_terrain(locs[i]);
		gui_.invalidate(locs[i]);
	}
	map_dirty_ = true;
}

void map_editor::invalidate_all_and_adjacent(const std::vector<gamemap::location> &hexes) {
	std::set<gamemap::location> to_invalidate;
	std::vector<gamemap::location>::const_iterator it;
	for (it = hexes.begin(); it != hexes.end(); it++) {
		terrain_changed(map_, *it);
		gamemap::location locs[7];
		locs[0] = *it;
		get_adjacent_tiles(*it, locs+1);
		for(int i = 0; i != 7; ++i) {
			to_invalidate.insert(locs[i]);
		}
	}
	std::set<gamemap::location>::const_iterator its;
	for (its = to_invalidate.begin(); its != to_invalidate.end(); its++) {
		if (!map_.on_board(*its)) {
			gamemap::TERRAIN terrain_before = map_.get_terrain(*its);
			map_.remove_from_border_cache(*its);
			gamemap::TERRAIN terrain_after = map_.get_terrain(*its);
			if (terrain_before != terrain_after) {
				invalidate_adjacent(*its);
			}
		}
		gui_.rebuild_terrain(*its);
		gui_.invalidate(*its);
	}
	map_dirty_ = true;
}

void map_editor::invalidate_all_and_adjacent(const std::set<gamemap::location> &hexes) {
	std::vector<gamemap::location> v;
	std::copy(hexes.begin(), hexes.end(), std::back_inserter(v));
	invalidate_all_and_adjacent(v);
}

void map_editor::right_button_down(const int /*mousex*/, const int /*mousey*/) {
}
void map_editor::middle_button_down(const int /*mousex*/, const int /*mousey*/) {
}


bool map_editor::confirm_exit_and_save() {
	if (gui::show_dialog(gui_, NULL, "Exit?",
						 "Do you want to exit the map editor?", gui::YES_NO) != 0) {
		return false;
	}
	if (changed_since_save() &&
		gui::show_dialog(gui_, NULL, "Save?",
						 "Do you want to save the map before exiting?", gui::YES_NO) == 0) {
		if (!save_map("", false)) {
			return false;
		}
	}
	return true;
}


bool map_editor::save_map(const std::string fn, const bool display_confirmation) {
	std::string filename = fn;
	if (filename == "") {
		filename = filename_;
	}
	else {
		filename_ = filename;
	}
	try {
		write_file(filename, map_.write());
		num_operations_since_save_ = 0;
		if (display_confirmation) {
			gui::show_dialog(gui_, NULL, "", "Map saved.", gui::OK_ONLY);
		}
	}
	catch (io_exception e) {
		std::string msg = "Could not save the map: "; 
		msg += e.what();
		gui::show_dialog(gui_, NULL, "", msg, gui::OK_ONLY);
		return false;
	}
	return true;
}

void map_editor::show_menu(const std::vector<std::string>& items_arg, const int xloc,
						   const int yloc, const bool /*context_menu*/) {
	std::vector<std::string> items = items_arg;
	// menu is what to display in the menu.
	std::vector<std::string> menu;
	if(items.size() == 1) {
		execute_command(hotkey::string_to_command(items.front()));
		return;
	}
	for(std::vector<std::string>::const_iterator i = items.begin();
		i != items.end(); ++i) {
		std::stringstream str;
		// Try to translate it to nicer format.
		str << translate_string("action_" + *i);
		// See if this menu item has an associated hotkey.
		const hotkey::HOTKEY_COMMAND cmd = hotkey::string_to_command(*i);
		const std::vector<hotkey::hotkey_item>& hotkeys = hotkey::get_hotkeys();
		std::vector<hotkey::hotkey_item>::const_iterator hk;
		for(hk = hotkeys.begin(); hk != hotkeys.end(); ++hk) {
			if(hk->action == cmd) {
				break;
			}
		}
		if(hk != hotkeys.end()) {
			// Hotkey was found for this item, add the hotkey description to
			// the menu item.
			str << "," << hotkey::get_hotkey_name(*hk);
		}
		menu.push_back(str.str());
	}
	static const std::string style = "menu2";
	const int res = gui::show_dialog(gui_, NULL, "", "", gui::MESSAGE, &menu, NULL, "",
									 NULL, NULL, NULL, xloc, yloc, &style);
	if(res < 0 || (unsigned)res >= items.size())
		return;
	const hotkey::HOTKEY_COMMAND cmd = hotkey::string_to_command(items[res]);
	execute_command(cmd);
}

void map_editor::execute_command(const hotkey::HOTKEY_COMMAND command) {
	if (command == hotkey::HOTKEY_QUIT_GAME) {
		set_abort();
	}
	else {
		hotkey::execute_command(gui_, command, this);
	}
}

void map_editor::recalculate_starting_pos_labels() {
	// Remove the current labels.
	for (std::vector<gamemap::location>::const_iterator it = starting_positions_.begin();
		 it != starting_positions_.end(); it++) {
		gui_.labels().set_label(*it, "");
	}
	starting_positions_.clear();
	// Set new labels.
	for (int i = 0; i < 10; i++) {
		gamemap::location loc = map_.starting_position(i);
		if (loc.valid()) {
			std::stringstream ss;
			ss << "Player " << i;
			gui_.labels().set_label(loc, ss.str());
			starting_positions_.push_back(loc);
		}
	}
}

void map_editor::update_mouse_over_hexes(const gamemap::location mouse_over_hex) {
	std::vector<gamemap::location> curr_locs =
		get_tiles(map_, mouse_over_hex, brush_.selected_brush_size());
	std::set<gamemap::location>::iterator it;
	for (it = mouse_over_hexes_.begin(); it != mouse_over_hexes_.end(); it++) {
		if (selected_hexes_.find(*it) == selected_hexes_.end()) {
			// Only remove highlightning if the hex is not selected in
			// an other way.
			gui_.remove_highlighted_loc(*it);
		}
	}
	mouse_over_hexes_.clear();
	if (mouse_over_hex != gamemap::location()) {
		// Only highlight if the mouse is on the map.
		for (std::vector<gamemap::location>::iterator itv = curr_locs.begin();
			 itv != curr_locs.end(); itv++) {
			mouse_over_hexes_.insert(*itv);
			gui_.add_highlighted_loc(*itv);
		}
	}
	gui_.highlight_hex(mouse_over_hex);
	selected_hex_ = mouse_over_hex;
}

void map_editor::main_loop() {
	unsigned int last_brush_size = brush_.selected_brush_size();
	while (abort_ == DONT_ABORT) {
		int mousex, mousey;
		const int scroll_speed = preferences::scroll_speed();
		const int mouse_flags = SDL_GetMouseState(&mousex,&mousey);
		const bool l_button_down = mouse_flags & SDL_BUTTON_LMASK;
		const bool r_button_down = mouse_flags & SDL_BUTTON_RMASK;
		const bool m_button_down = mouse_flags & SDL_BUTTON_MMASK;

		const gamemap::location cur_hex = gui_.hex_clicked_on(mousex,mousey);
		if (cur_hex != selected_hex_) {
			mouse_moved_ = true;
		}
		if (mouse_moved_ || last_brush_size != brush_.selected_brush_size()) {
			// The mouse have moved or the brush size has changed,
			// adjust the hexes the mouse is over.
			update_mouse_over_hexes(cur_hex);
			last_brush_size = brush_.selected_brush_size();
		}
		const theme::menu* const m = gui_.menu_pressed(mousex, mousey,
													   mouse_flags & SDL_BUTTON_LMASK);
		if (m != NULL) {
			const SDL_Rect& menu_loc = m->location(gui_.screen_area());
			const int x = menu_loc.x + 1;
			const int y = menu_loc.y + menu_loc.h + 1;
			show_menu(m->items(), x, y, false);
		}
	
		if(key_[SDLK_UP] || mousey == 0) {
			gui_.scroll(0,-scroll_speed);
		}
		if(key_[SDLK_DOWN] || mousey == gui_.y()-1) {
			gui_.scroll(0,scroll_speed);
		}
		if(key_[SDLK_LEFT] || mousex == 0) {
			gui_.scroll(-scroll_speed,0);
		}
		if(key_[SDLK_RIGHT] || mousex == gui_.x()-1) {
			gui_.scroll(scroll_speed,0);
		}
	
		if (l_button_down) {
			left_button_down(mousex, mousey);
		}
		else {
			if (l_button_func_ == MOVE_SELECTION) {
				// When it is detected that the mouse is no longer down
				// and we are in the progress of moving a selection,
				// perform the movement.
				perform_selection_move();
			}
		}
		if (r_button_down) {
			right_button_down(mousex, mousey);
		}
		if (m_button_down) {
			middle_button_down(mousex, mousey);
		}

		gui_.draw(false);
		events::raise_draw_event();

		// When the map has changed, wait until the left mouse button is
		// not held down and then update the minimap and the starting
		// position labels.
		if (map_dirty_) {
			if (!l_button_down) {
				map_dirty_ = false;
				recalculate_starting_pos_labels();
				gui_.recalculate_minimap();
			}
		}
		gui_.update_display();
		SDL_Delay(sdl_delay);
		events::pump();
		if (abort_ == ABORT_NORMALLY) {
			if (!confirm_exit_and_save()) {
				set_abort(DONT_ABORT);
			}
		}
		mouse_moved_ = false;
	}
}


}


