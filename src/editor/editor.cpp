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
#include "../pathfind.hpp"
#include "../playlevel.hpp"
#include "../preferences.hpp"
#include "../sdl_utils.hpp"
#include "../tooltips.hpp"
#include "../team.hpp"
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
	const int num_players = 10;
	// Milliseconds to sleep in every iteration of the main loop.
	const unsigned int sdl_delay = 20;
	const std::string prefs_filename = get_dir(get_user_data_dir() + "/editor")
		+ "/preferences";

	// Return the side that has it's starting position at hex, or -1 if
	// none has.
	int starting_side_at(const gamemap& map, const gamemap::location hex) {
		int start_side = -1;
		for (int i = 0; i < 10; i++) {
			if (map.starting_position(i) == hex) {
				start_side = i;
			}
		}
		return start_side;
	}
}

namespace map_editor {

// The map_editor object will be recreated when operations that affect
// the whole map takes place. It may not be the most beautiful solution,
// but it is the way the least interference with the game system is
// needed. That is the reason we need some static variables to handle
// things that should be permanent through the program's life time. Of
// course, the functionality of this assumes that no more than one
// map_editor object will exist, but that is a reasonable restriction
// imho.
bool map_editor::first_time_created_ = true;
int map_editor::num_operations_since_save_ = 0;
config map_editor::prefs_;
config map_editor::hotkeys_;
// Do not init the l_button_func_ to DRAW, since it should be changed in
// the constructor to update the report the first time.
map_editor::LEFT_BUTTON_FUNC map_editor::l_button_func_ = PASTE; 
gamemap::TERRAIN map_editor::old_fg_terrain_;
gamemap::TERRAIN map_editor::old_bg_terrain_;
int map_editor::old_brush_size_;

map_editor::map_editor(display &gui, gamemap &map, config &theme, config &game_config)
	: gui_(gui), map_(map), abort_(DONT_ABORT),
	  theme_(theme), game_config_(game_config), map_dirty_(false), l_button_palette_dirty_(true),
	  everything_dirty_(false), palette_(gui, size_specs_, map), brush_(gui, size_specs_),
	  l_button_held_func_(NONE), highlighted_locs_cleared_(false), prefs_disp_manager_(&gui_),
	  all_hexes_selected_(false) {

	// Set size specs.
	adjust_sizes(gui_, size_specs_);
	if (first_time_created_) {
		// Perform some initializations that should only be performed
		// the first time the editor object is created.
		try {
			prefs_.read(read_file(prefs_filename));
		}
		catch (config::error e) {
			std::cerr << "Error when reading " << prefs_filename << ": "
					  << e.message << std::endl;
		}
		hotkey::load_hotkeys(theme_);
		hotkey::load_hotkeys(prefs_);
		left_button_func_changed(DRAW);
		first_time_created_ = false;
	}
	else {
		hotkey::load_hotkeys(hotkeys_);
		palette_.select_fg_terrain(old_fg_terrain_);
		palette_.select_bg_terrain(old_bg_terrain_);
		brush_.select_brush_size(old_brush_size_);
	}
	hotkey::load_descriptions();
	recalculate_starting_pos_labels();
	gui_.invalidate_game_status();
	gui_.begin_game();
	gui_.invalidate_all();
	gui_.draw();
	palette_.adjust_size();
	brush_.adjust_size();
	events::raise_draw_event();
	redraw_everything();
}

map_editor::~map_editor() {
	// Save the hotkeys so that they are remembered if the editor is recreated.
	hotkey::save_hotkeys(hotkeys_);
	old_fg_terrain_ = palette_.selected_fg_terrain();
	old_bg_terrain_ = palette_.selected_bg_terrain();
	old_brush_size_ = brush_.selected_brush_size();
	try {
		write_file(prefs_filename, prefs_.write());
	}
	catch (io_exception& e) {
		std::cerr << "Error when writing to " << prefs_filename << ": "
				  << e.what() << std::endl;
	}
}

void map_editor::handle_event(const SDL_Event &event) {
	if (event.type == SDL_VIDEORESIZE) {
		everything_dirty_ = true;
	}
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
				everything_dirty_ = true;
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
			if (m != NULL) {
				show_menu(m->items(), mousex, mousey + 1, true);
			}
		}
		if (button == SDL_BUTTON_LEFT) {
			gamemap::location hex_clicked = gui_.hex_clicked_on(mousex, mousey);
			if (map_.on_board(hex_clicked)) {
				left_click(hex_clicked);
			}
		}
		if (button == SDL_BUTTON_RIGHT) {
			gamemap::location hex_clicked = gui_.hex_clicked_on(mousex, mousey);
			if (map_.on_board(hex_clicked)) {
				right_click(hex_clicked);
			}
		}
		// Mimic the game's behavior on middle click and mouse wheel
		// movement. It would be nice to have had these in functions
		// provided from the game, but I don't want to interfer too much
		// with the game code and it's fairly simple stuff to rip.
		if (button == SDL_BUTTON_MIDDLE) {
			const SDL_Rect& rect = gui_.map_area();
			const int centerx = (rect.x + rect.w) / 2;
			const int centery = (rect.y + rect.h) / 2;
			
			const int xdisp = mousex - centerx;
			const int ydisp = mousey - centery;
			gui_.scroll(xdisp, ydisp);
		}
		if(button == SDL_BUTTON_WHEELUP ||
		   button == SDL_BUTTON_WHEELDOWN) {
			if (point_in_rect(mousex, mousey, gui_.map_area())) {
				const int speed = preferences::scroll_speed() *
					(button == SDL_BUTTON_WHEELUP ? -1 : 1);
				
				const int centerx = gui_.mapx() / 2;
				const int centery = gui_.y() / 2;
				
				const int xdisp = abs(centerx - mousex);
				const int ydisp = abs(centery - mousey);
				
				if(xdisp > ydisp)
					gui_.scroll(speed,0);
				else
					gui_.scroll(0,speed);
			}
		}
	}
	if (event.type == SDL_MOUSEBUTTONUP) {
		// If we miss the mouse up event we need to perform the actual
		// movement if we are in the progress of moving a selection.
		if (l_button_held_func_ == MOVE_SELECTION) {
			perform_selection_move();
		}
		l_button_held_func_ = NONE;
	}
}

void map_editor::left_click(const gamemap::location hex_clicked) {
	if (key_[SDLK_LSHIFT] || key_[SDLK_RSHIFT]) {
		if (selected_hexes_.find(hex_clicked)
			== selected_hexes_.end()) {
			l_button_held_func_ = ADD_SELECTION;
		}
		else {
			l_button_held_func_ = REMOVE_SELECTION;
		}
	}
	else if (key_[SDLK_RCTRL] || key_[SDLK_LCTRL]) {
		const gamemap::TERRAIN terrain = map_.get_terrain(selected_hex_);
		if(palette_.selected_fg_terrain() != terrain) {
			palette_.select_fg_terrain(terrain);
		}
		l_button_held_func_ = NONE;
	}
	else if (selected_hexes_.find(hex_clicked) != selected_hexes_.end()) {
		l_button_held_func_ = MOVE_SELECTION;
		selection_move_start_ = hex_clicked;
	}
	else if (!selected_hexes_.empty()) {
		// If hexes are selected, clear them and do not draw
		// anything.
		selected_hexes_.clear();
		clear_highlighted_hexes_in_gui();
	}
	else if (l_button_func_ == DRAW) {
		l_button_held_func_ = DRAW_TERRAIN;
	}
	else if (l_button_func_ == FLOOD_FILL) {
		perform_flood_fill();
	}
	else if (l_button_func_ == SET_STARTING_POSITION) {
		perform_set_starting_pos();
	}
	else if (l_button_func_ == PASTE) {
		perform_paste();
	}
}

void map_editor::right_click(const gamemap::location hex_clicked ) {
	if (key_[SDLK_RCTRL] || key_[SDLK_LCTRL]) {
		const gamemap::TERRAIN terrain = map_.get_terrain(hex_clicked);
		if(palette_.selected_fg_terrain() != terrain) {
			palette_.select_bg_terrain(terrain);
		}
	}
}
	

void map_editor::edit_save_as() {
	const std::string default_dir =
		get_dir(get_dir(get_user_data_dir() + "/editor") + "/maps/");
	std::string input_name = filename_.empty() ? default_dir : filename_;
	int res = 0;
	int overwrite = 0;
	do {
		res = dialogs::show_file_chooser_dialog(gui_, input_name, _("Save the Map As"));
		if (res == 0) {
			if (file_exists(input_name)) {
				overwrite = gui::show_dialog(gui_, NULL, "",
					_("The map already exists. Do you want to overwrite it?"),
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

void map_editor::perform_set_starting_pos() {
	std::vector<std::string> players;
	for (int i = 0; i < num_players; i++) {
		std::stringstream str;
		str << "Player " << i;
		players.push_back(str.str());
	}
	const int res = gui::show_dialog(gui_, NULL, _("Which Player?"),
	                                 _("Which player should start here?"),
	                                 gui::OK_CANCEL, &players);
	if (res >= 0) {
		set_starting_position(res, selected_hex_);
	}
}

void map_editor::edit_set_start_pos() {
	left_button_func_changed(SET_STARTING_POSITION);
}

void map_editor::perform_flood_fill() {
	terrain_log log;
	flood_fill(map_, selected_hex_, palette_.selected_fg_terrain(), &log);
	std::vector<gamemap::location> to_invalidate;
	map_undo_action action;
	for (terrain_log::iterator it = log.begin(); it != log.end(); it++) {
		to_invalidate.push_back((*it).first);
		action.add_terrain((*it).second, palette_.selected_fg_terrain(),
			   (*it).first);
	}
	terrain_changed(to_invalidate, action);
	save_undo_action(action);
}

void map_editor::edit_flood_fill() {
	left_button_func_changed(FLOOD_FILL);
}

void map_editor::edit_save_map() {
	save_map("", true);
}

void map_editor::edit_quit() {
	set_abort();
}

void map_editor::edit_new_map() {
	const std::string map = new_map_dialog(gui_, palette_.selected_bg_terrain(),
	                                       changed_since_save(), game_config_);
 	if (map != "") {
		num_operations_since_save_ = 0;
		clear_undo_actions();
		throw new_map_exception(map);
	}
}

void map_editor::edit_load_map() {
	std::string fn = filename_.empty() ?
		get_dir(get_dir(get_user_data_dir() + "/editor") + "/maps/") : filename_;
	int res = dialogs::show_file_chooser_dialog(gui_, fn, _("Choose a Map to Load"));
	if (res == 0) {
		std::string new_map;
		try {
			new_map = load_map(fn);
		}
		catch (load_map_exception) {
			return;
		}
		if (valid_mapdata(new_map, game_config_)) {
			if (!changed_since_save() || confirm_modification_disposal(gui_)) {
				num_operations_since_save_ = 0;
				clear_undo_actions();
				throw new_map_exception(new_map, fn);
			}
		}
		else {
			gui::show_dialog(gui_, NULL, "", _("The file does not contain a valid map."), gui::OK_ONLY);
		}
	}
}

void map_editor::edit_fill_selection() {
	map_undo_action undo_action;
	for (std::set<gamemap::location>::const_iterator it = selected_hexes_.begin();
		 it != selected_hexes_.end(); it++) {
		if (map_.on_board(*it)) {
			undo_action.add_terrain(map_.get_terrain(*it), palette_.selected_fg_terrain(), *it);
			map_.set_terrain(*it, palette_.selected_fg_terrain());
		}
	}
	terrain_changed(selected_hexes_, undo_action);
	save_undo_action(undo_action);
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

void map_editor::perform_paste() {
	map_undo_action undo_action;
	std::set<gamemap::location> filled;
	gamemap::location start_hex = selected_hex_;
	clear_highlighted_hexes_in_gui();
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
			const int start_side = (*it).starting_side;
			if (start_side != -1) {
				undo_action.add_starting_location(start_side, start_side,
					map_.starting_position(start_side), target);
				map_.set_starting_position(start_side, target);
			}
			filled.insert(target);
			gui_.add_highlighted_loc(target);
		}
	}
	undo_action.set_selection(selected_hexes_, filled);
	terrain_changed(filled, undo_action);
	selected_hexes_ = filled;
	save_undo_action(undo_action);
}

void map_editor::edit_paste() {
	left_button_func_changed(PASTE);
}

void map_editor::edit_revert() {
	std::string new_map;
	try {
		new_map = load_map(filename_);
	}
	catch (load_map_exception) {
		return;
	}
	if (valid_mapdata(new_map, game_config_)) {
		map_undo_action action;
		action.set_map_data(map_.write(), new_map);
		save_undo_action(action);
		throw new_map_exception(new_map, filename_);
	}
	else {
		gui::show_dialog(gui_, NULL, "", _("The file does not contain a valid map."), gui::OK_ONLY);
	}
}

void map_editor::edit_resize() {
	const std::pair<unsigned, unsigned> new_size =
		resize_dialog(gui_, map_.x(), map_.y());
	if (new_size.first != 0) {
		const std::string resized_map =
			resize_map(map_, new_size.first, new_size.second, palette_.selected_bg_terrain());
		if (resized_map != "") {
			map_undo_action action;
			action.set_map_data(map_.write(), resized_map);
			save_undo_action(action);
			throw new_map_exception(resized_map, filename_);
		}
	}
}

void map_editor::edit_flip() {
	const FLIP_AXIS flip_axis = flip_dialog(gui_);
	if (flip_axis != NO_FLIP) {
		const std::string flipped_map = flip_map(map_, flip_axis);
		map_undo_action action;
		action.set_map_data(map_.write(), flipped_map);
		save_undo_action(action);
		throw new_map_exception(flipped_map, filename_);
	}
}

void map_editor::edit_select_all() {
	if (!all_hexes_selected_) {
		for (int i = 0; i < map_.x(); i++) {
			for (int j = 0; j < map_.y(); j++) {
				selected_hexes_.insert(gamemap::location(i, j));
			}
		}
		all_hexes_selected_ = true;
	}
	else {
		selected_hexes_.clear();
		all_hexes_selected_ = false;
	}
	highlight_selected_hexes();
}

void map_editor::edit_draw() {
	left_button_func_changed(DRAW);
}

std::string map_editor::load_map(const std::string filename) {
	bool load_successful = true;
	std::string msg;
	std::string new_map;
	string_map symbols;
	symbols["filename"] = filename;
	if (!file_exists(filename) || is_directory(filename)) {
		load_successful = false;
		msg = vgettext("'$filename' does not exist or can not be read as a file.", symbols);
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
		const std::string failed_msg = _("Load failed: ");
		const std::string show_msg = failed_msg +
			config::interpolate_variables_into_string(msg, &symbols);
		gui::show_dialog(gui_, NULL, "", show_msg, gui::OK_ONLY);
		throw load_map_exception();
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
		if (distance_between(selected_hex_, *it) <
			distance_between(selected_hex_, offset_hex)) {
			offset_hex = *it;
		}
	}
	clipboard_offset_loc_ = offset_hex;
	for (it = selected_hexes_.begin(); it != selected_hexes_.end(); it++) {
		const int x_offset = (*it).x - offset_hex.x;
		const int y_offset = (*it).y - offset_hex.y;
		gamemap::TERRAIN terrain = map_.get_terrain(*it);
		clipboard_.push_back(clipboard_item(x_offset, y_offset, terrain,
		                     starting_side_at(map_, *it)));
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
	case hotkey::HOTKEY_EDIT_FLIP:
	case hotkey::HOTKEY_EDIT_SELECT_ALL:
	case hotkey::HOTKEY_EDIT_DRAW:
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

void map_editor::save_undo_action(const map_undo_action &action) {
	add_undo_action(action);
	num_operations_since_save_++;
}

void map_editor::undo() {
	if(exist_undo_actions()) {
		--num_operations_since_save_;
		std::vector<gamemap::location> to_invalidate;
		map_undo_action action = pop_undo_action();
		if (action.selection_set()) {
			selected_hexes_ = action.undo_selection();
			highlight_selected_hexes(true);
			std::copy(selected_hexes_.begin(), selected_hexes_.end(),
					  std::back_inserter(to_invalidate));
		}
		if (action.terrain_set()) {
			for(std::map<gamemap::location,gamemap::TERRAIN>::const_iterator it =
					action.undo_terrains().begin();
				it != action.undo_terrains().end(); ++it) {
				map_.set_terrain(it->first, it->second);
				to_invalidate.push_back(it->first);
			}
		}
		if (action.starting_location_set()) {
			for (std::map<gamemap::location, int>::const_iterator it =
					 action.undo_starting_locations().begin();
				 it != action.undo_starting_locations().end(); it++) {
				map_.set_starting_position((*it).second, (*it).first);
				to_invalidate.push_back((*it).first);
			}
		}
		invalidate_all_and_adjacent(to_invalidate);
		if (action.map_data_set()) {
			throw new_map_exception(action.old_map_data(), filename_);
		}
	}
}

void map_editor::redo() {
	if(exist_redo_actions()) {
		++num_operations_since_save_;
		map_undo_action action = pop_redo_action();
		std::vector<gamemap::location> to_invalidate;
		if (action.selection_set()) {
			selected_hexes_ = action.redo_selection();
			highlight_selected_hexes(true);
			std::copy(selected_hexes_.begin(), selected_hexes_.end(),
					  std::back_inserter(to_invalidate));
		}
		if (action.terrain_set()) {
			for(std::map<gamemap::location,gamemap::TERRAIN>::const_iterator it =
					action.redo_terrains().begin();
				it != action.redo_terrains().end(); ++it) {
				map_.set_terrain(it->first, it->second);
				to_invalidate.push_back(it->first);
			}
		}
		if (action.starting_location_set()) {
			for (std::map<gamemap::location, int>::const_iterator it =
					 action.redo_starting_locations().begin();
				 it != action.redo_starting_locations().end(); it++) {
				map_.set_starting_position((*it).second, (*it).first);
				to_invalidate.push_back((*it).first);
			}
		}
		invalidate_all_and_adjacent(to_invalidate);
		 if (action.map_data_set()) {
			throw new_map_exception(action.new_map_data(), filename_);
		}
	}
}

void map_editor::preferences() {
	preferences_dialog(gui_, prefs_);
	everything_dirty_ = true;
}

void map_editor::redraw_everything() {
	adjust_sizes(gui_, size_specs_);
	palette_.adjust_size();
	brush_.adjust_size();
	gui_.redraw_everything();
	update_l_button_palette();
	palette_.draw(true);
	brush_.draw(true);
}

void map_editor::highlight_selected_hexes(const bool clear_old) {
	if (clear_old) {
		clear_highlighted_hexes_in_gui();
	}
	for (std::set<gamemap::location>::const_iterator it = selected_hexes_.begin();
		 it != selected_hexes_.end(); it++) {
		gui_.add_highlighted_loc(*it);
	}
}

void map_editor::clear_highlighted_hexes_in_gui() {
	gui_.clear_highlighted_locs();
	highlighted_locs_cleared_ = true;
}

bool map_editor::changed_since_save() const {
	return num_operations_since_save_ != 0;
}

void map_editor::set_starting_position(const int player, const gamemap::location loc) {
	if(map_.on_board(loc)) {
		map_undo_action action;
		action.add_terrain(map_.get_terrain(selected_hex_), gamemap::KEEP,
									selected_hex_);
		map_.set_terrain(selected_hex_, gamemap::KEEP);
		terrain_changed(selected_hex_, action);
		action.add_starting_location(player, player, map_.starting_position(player), loc);
		map_.set_starting_position(player, loc);
		save_undo_action(action);
	}
	else {
		gui::show_dialog(gui_, NULL, "",
		                 _("You must have a hex selected on the board."), gui::OK_ONLY);
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
				if (l_button_held_func_ == ADD_SELECTION) {
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
				if (l_button_held_func_ == ADD_SELECTION) {
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
	else if (l_button_held_func_ == DRAW_TERRAIN) {
		draw_on_mouseover_hexes(palette_.selected_fg_terrain());
	}
	else if (l_button_held_func_ == MOVE_SELECTION) {
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

void map_editor::draw_on_mouseover_hexes(const gamemap::TERRAIN terrain) {
	const gamemap::location hex = selected_hex_;
	if(map_.on_board(hex)) {
		const gamemap::TERRAIN old_terrain = map_[hex.x][hex.y];
		// optimize for common case
		if(brush_.selected_brush_size() == 1) {
			if(terrain != old_terrain) {
				draw_terrain(terrain, hex);
			}
		}
		else {
			std::vector<gamemap::location> locs =
				get_tiles(map_, hex, brush_.selected_brush_size());
			map_undo_action action;
			std::vector<gamemap::location> to_invalidate;
			for(std::vector<gamemap::location>::const_iterator it = locs.begin();
				it != locs.end(); ++it) {
				if(terrain != map_[it->x][it->y]) {
					to_invalidate.push_back(*it);
					action.add_terrain(map_.get_terrain(*it), terrain, *it);
					map_.set_terrain(*it, terrain);
					gui_.rebuild_terrain(*it);
				}
			}
			if (!to_invalidate.empty()) {
				terrain_changed(to_invalidate, action);
				save_undo_action(action);
			}
		}
	}
}

void map_editor::perform_selection_move() {
	map_undo_action undo_action;
	const int x_diff = selected_hex_.x - selection_move_start_.x;
	const int y_diff = selected_hex_.y - selection_move_start_.y;
	clear_highlighted_hexes_in_gui();
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
			const int start_side = starting_side_at(map_, *it);
			if (start_side != -1) {
				// Starting side at the old location that needs transfering.
				map_.set_starting_position(start_side, hl_loc);
				undo_action.add_starting_location(start_side, start_side, *it, hl_loc);
			}
			new_selection.insert(hl_loc);
		}
	}

	// Fill the selection with the selected terrain.
	for (it = selected_hexes_.begin(); it != selected_hexes_.end(); it++) {
		if (map_.on_board(*it) && new_selection.find(*it) == new_selection.end()) {
			undo_action.add_terrain(map_.get_terrain(*it), palette_.selected_bg_terrain(), *it);
			map_.set_terrain(*it, palette_.selected_bg_terrain());
		}
	}
	undo_action.set_selection(selected_hexes_, new_selection);
	terrain_changed(selected_hexes_, undo_action);
	selected_hexes_ = new_selection;
	terrain_changed(selected_hexes_, undo_action);
	save_undo_action(undo_action);
}

void map_editor::draw_terrain(const gamemap::TERRAIN terrain, const gamemap::location hex) {
	const gamemap::TERRAIN current_terrain = map_.get_terrain(hex);
	map_undo_action undo_action;
	undo_action.add_terrain(current_terrain, terrain, hex);
	map_.set_terrain(hex, terrain);
	gui_.rebuild_terrain(hex);
	terrain_changed(hex, undo_action);
	save_undo_action(undo_action);
}

void map_editor::terrain_changed(const gamemap::location &hex, map_undo_action &undo_action) {
	std::vector<gamemap::location> v;
	v.push_back(hex);
	terrain_changed(v, undo_action);
}

void map_editor::terrain_changed(const std::vector<gamemap::location> &hexes,
								 map_undo_action &undo_action) {
	for (std::vector<gamemap::location>::const_iterator it = hexes.begin();
		 it != hexes.end(); it++) {
		const int start_side = starting_side_at(map_, *it);
		if (start_side != -1 && map_.get_terrain(*it) != gamemap::KEEP) {
			// A terrain which had a starting position has changed, save
			// this position in the undo_action and unset it.
			map_.set_starting_position(start_side, gamemap::location());
			undo_action.add_starting_location(start_side, start_side, *it, gamemap::location());
		}
	}
	invalidate_all_and_adjacent(hexes);
}

void map_editor::terrain_changed(const std::set<gamemap::location> &hexes,
								 map_undo_action &undo_action) {
	std::vector<gamemap::location> v;
	std::copy(hexes.begin(), hexes.end(), std::back_inserter(v));
	terrain_changed(v, undo_action);
}


void map_editor::invalidate_adjacent(const gamemap::location hex) {
	std::set<gamemap::location> s;
	s.insert(hex);
	invalidate_all_and_adjacent(s);
}

void map_editor::invalidate_all_and_adjacent(const std::vector<gamemap::location> &hexes) {
	std::set<gamemap::location> to_invalidate;
	std::vector<gamemap::location>::const_iterator it;
	for (it = hexes.begin(); it != hexes.end(); it++) {
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
		gui_.invalidate(*its);
	}
	map_dirty_ = true;
}

void map_editor::invalidate_all_and_adjacent(const std::set<gamemap::location> &hexes) {
	std::vector<gamemap::location> v;
	std::copy(hexes.begin(), hexes.end(), std::back_inserter(v));
	invalidate_all_and_adjacent(v);
}

void map_editor::right_button_down(const int mousex, const int mousey) {
	// Draw with the bacground terrain on rightclick, no matter what
	// operations are wanted with the left button.
	// TODO evaluate if this is what is the smartest thing to do.
	draw_on_mouseover_hexes(palette_.selected_bg_terrain());
}
 
void map_editor::middle_button_down(const int mousex, const int mousey) {
	const gamemap::location& minimap_loc = gui_.minimap_location_on(mousex,mousey);
	const gamemap::location hex = gui_.hex_clicked_on(mousex, mousey);
	if (minimap_loc.valid()) {
		gui_.scroll_to_tile(minimap_loc.x,minimap_loc.y,display::WARP,false);
	}
}

bool map_editor::confirm_exit_and_save() {
	if (gui::show_dialog(gui_, NULL, "",
	                     _("Quit Editor"), gui::YES_NO) != 0) {
		return false;
	}
	if (changed_since_save() &&
	    gui::show_dialog(gui_, NULL, "",
		             _("Do you want to save the map before quitting?"), gui::YES_NO) == 0) {
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
		if(filename == "") {
			edit_save_as();
			return true;
		}
	}
	else {
		filename_ = filename;
	}
	try {
		write_file(filename, map_.write());
		num_operations_since_save_ = 0;
		if (display_confirmation) {
			gui::show_dialog(gui_, NULL, "", _("Map saved."),
							 gui::OK_ONLY);
		}
	}
	catch (io_exception& e) {
		string_map symbols;
		symbols["msg"] = e.what();
		const std::string msg = vgettext("Could not save the map: $msg",symbols);
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
		execute_command(hotkey::get_hotkey(items.front()).get_id());
		return;
	}
	for(std::vector<std::string>::const_iterator i = items.begin();
		i != items.end(); ++i) {

		const hotkey::hotkey_item hk = hotkey::get_hotkey(*i);
		std::stringstream str;
		// Try to translate it to nicer format.
		str << hk.get_description() << COLUMN_SEPARATOR << hk.get_name();

		menu.push_back(str.str());
	}
	static const std::string style = "menu2";
	const int res = gui::show_dialog(gui_, NULL, "", "", gui::MESSAGE, &menu, NULL, "",
	                                 NULL, 256, NULL, NULL, xloc, yloc, &style);
	if(res < 0 || (unsigned)res >= items.size())
		return;
	const hotkey::HOTKEY_COMMAND cmd = hotkey::get_hotkey(items[res]).get_id();
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
			ss << _("Player") << i;
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

void map_editor::left_button_func_changed(const LEFT_BUTTON_FUNC func) {
	if (func != l_button_func_) {
		l_button_func_ = func;
		reports::set_report_content(reports::EDIT_LEFT_BUTTON_FUNCTION,
				hotkey::get_hotkey(get_action_name(func)).get_description());
		gui_.invalidate_game_status();
		l_button_palette_dirty_ = true;
	}
}

void map_editor::update_l_button_palette() {
	const theme &t = gui_.get_theme();
	const std::vector<theme::menu> &menus = t.menus();
	std::vector<theme::menu>::const_iterator it;
	for (it = menus.begin(); it != menus.end(); it++) {
		if (is_left_button_func_menu(*it)) {
			const SDL_Rect &r = (*it).location(gui_.screen_area());
			const int draw_x = maximum<int>(r.x - 1, gui_.screen_area().x);
			const int draw_y = maximum<int>(r.y - 1, gui_.screen_area().y);
			const int draw_w = minimum<int>(r.w + 2, gui_.screen_area().w);
			const int draw_h = minimum<int>(r.h + 2, gui_.screen_area().h);
			const SDL_Rect draw_rect = {draw_x, draw_y, draw_w, draw_h};
			SDL_Surface* const screen = gui_.video().getSurface();
			Uint32 color;
			if ((*it).items().back() == get_action_name(l_button_func_)) {
				color = SDL_MapRGB(screen->format,0xFF,0x00,0x00);
			}
			else {
				color = SDL_MapRGB(screen->format,0x00,0x00,0x00);
			}
			gui::draw_rectangle(draw_rect.x, draw_rect.y, draw_rect.w, draw_rect.h,
								color, gui_.video().getSurface());
			update_rect(draw_rect);
		}
	}
}

std::string map_editor::get_action_name(const LEFT_BUTTON_FUNC func) const {
	switch (func) {
	case DRAW:
		return "editdraw";
	case SELECT_HEXES:
		return ""; // Not implemented yet.
	case FLOOD_FILL:
		return "editfloodfill";
	case SET_STARTING_POSITION:
		return "editsetstartpos";
	case PASTE:
		return "editpaste";
	default:
		return "";
	}
}

bool map_editor::is_left_button_func_menu(const theme::menu &menu) const {
	const std::vector<std::string> &menu_items = menu.items();
	if (menu_items.size() == 1) {
		const std::string item_name = menu_items.back();
		for (size_t i = 0; i < NUM_L_BUTTON_FUNC; i++) {
			if (get_action_name(LEFT_BUTTON_FUNC(i)) == item_name) {
				return true;
			}
		}
	}
	return false;
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
		if (mouse_moved_ || last_brush_size != brush_.selected_brush_size()
			|| highlighted_locs_cleared_) {
			// The mouse have moved or the brush size has changed,
			// adjust the hexes the mouse is over.
			highlighted_locs_cleared_ = false;
			update_mouse_over_hexes(cur_hex);
			last_brush_size = brush_.selected_brush_size();
		}
		const theme::menu* const m = gui_.menu_pressed();
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
			if (l_button_held_func_ == MOVE_SELECTION) {
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
			if (!l_button_down && !r_button_down) {
				map_dirty_ = false;
				gui_.rebuild_all();
				gui_.invalidate_all();
				recalculate_starting_pos_labels();
				gui_.recalculate_minimap();
			}
		}
		if (l_button_palette_dirty_) {
			update_l_button_palette();
			l_button_palette_dirty_ = false;
		}
		gui_.update_display();
		SDL_Delay(sdl_delay);
		events::pump();
		if (everything_dirty_) {
			redraw_everything();
			everything_dirty_ = false;
		}
		if (abort_ == ABORT_NORMALLY) {
			if (!confirm_exit_and_save()) {
				set_abort(DONT_ABORT);
			}
		}
		mouse_moved_ = false;
	}
}


}


