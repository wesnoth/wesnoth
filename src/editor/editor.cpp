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

namespace {
	const unsigned int undo_limit = 100;
	// Milliseconds to sleep in every iteration of the main loop.
	const unsigned int sdl_delay = 20;

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
}

namespace map_editor {
map_editor::map_editor(display &gui, gamemap &map, config &theme, config &game_config)
	: gui_(gui), map_(map), tup_(gui, "", gui::button::TYPE_PRESS, "uparrow-button"),
	  tdown_(gui, "", gui::button::TYPE_PRESS, "downarrow-button"), abort_(DONT_ABORT),
	  num_operations_since_save_(0), theme_(theme), game_config_(game_config),
	  draw_terrain_(false), map_dirty_(false),
	  palette_(gui, size_specs_, map), brush_(gui, size_specs_), add_selection_(false) {


	// Set size specs.
	adjust_sizes(gui_, size_specs_, palette_.num_terrains());

	tup_.set_xy(gui.mapx() + size_specs_.button_x, size_specs_.top_button_y);
	tdown_.set_xy(gui.mapx() + size_specs_.button_x, size_specs_.bot_button_y);

	// Clear the current hotkeys. Alot of hotkeys are already set
	// through other configuration files (e.g. english.cfg) and we need
	// to clear these or they will overlap.
	hotkey::get_hotkeys().clear();
	hotkey::add_hotkeys(theme_, true);
	recalculate_starting_pos_labels();
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
									   const int mousex, const int mousey) {
	if (event.type == SDL_KEYDOWN) {
		const SDLKey sym = event.keysym.sym;
		// We must intercept escape-presses here because we don't want the
		// default shutdown behavior, we want to ask for saving.
		if (sym == SDLK_ESCAPE) {
			set_abort();
		}
		else {
			hotkey::key_event(gui_, event, this);
		}
	}
}

void map_editor::handle_mouse_button_event(const SDL_MouseButtonEvent &event,
										   const int mousex, const int mousey) {
	if (event.type == SDL_MOUSEBUTTONDOWN) {
		const Uint8 button = event.button;
		if (button == SDL_BUTTON_WHEELUP) {
			palette_.scroll_up();
		}
		if (button == SDL_BUTTON_WHEELDOWN) {
			palette_.scroll_down();
		}
		if (button == SDL_BUTTON_RIGHT) {
			selected_hex_ = gui_.hex_clicked_on(mousex, mousey);
			const theme::menu* const m = gui_.get_theme().context_menu();
			show_menu(m->items(), mousex, mousey + 1, true);
		}
		if (button == SDL_BUTTON_LEFT) {
			if (key_[SDLK_LSHIFT] || key_[SDLK_RSHIFT]) {
				// Set a boolean to indicate wether to add or remove selected hexes.
				if (selected_hexes_.find(gui_.hex_clicked_on(mousex, mousey))
					== selected_hexes_.end()) {
					add_selection_ = true;
				}
				else {
					add_selection_ = false;
				}
			}
			else if (!selected_hexes_.empty()
					 && map_.on_board(gui_.hex_clicked_on(mousex, mousey))
				&& !(key_[SDLK_LSHIFT] || key_[SDLK_RSHIFT])) {
				// If hexes are selected, clear them and do not draw
				// anything.
				selected_hexes_.clear();
				gui_.clear_highlighted_locs();
				update_mouse_over_hexes(gui_.hex_clicked_on(mousex, mousey));
			}
			else {
				draw_terrain_ = true;
				palette_.left_mouse_click(mousex, mousey);
				brush_.left_mouse_click(mousex, mousey);
			}
		}
	}
	if (event.type == SDL_MOUSEBUTTONUP) {
		draw_terrain_ = false;
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
	num_operations_since_save_++;
	for (terrain_log::iterator it = log.begin(); it != log.end(); it++) {
		to_invalidate.push_back((*it).first);
		action.add((*it).second, palette_.selected_terrain(),
			   (*it).first);
	}
	add_undo_action(action);
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
		throw new_map_exception(map);
	}
}

void map_editor::edit_load_map() {
	const std::string map = load_map_dialog(gui_, changed_since_save());
	if (map != "") {
		throw new_map_exception(map);
	}
}

void map_editor::edit_fill_selection() {
	map_undo_action undo_action;
	for (std::set<gamemap::location>::const_iterator it = selected_hexes_.begin();
		 it != selected_hexes_.end(); it++) {
		if (map_.on_board(*it)) {
			undo_action.add(map_.get_terrain(*it), palette_.selected_terrain(), *it);
			map_.set_terrain(*it, palette_.selected_terrain());
		}
	}
	add_undo_action(undo_action);
	invalidate_all_and_adjacent(selected_hexes_);
}

void map_editor::edit_cut() {

}

void map_editor::edit_copy() {

}

void map_editor::edit_paste() {

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
	case hotkey::HOTKEY_EDIT_SAVE_MAP:
	case hotkey::HOTKEY_EDIT_SAVE_AS:
	case hotkey::HOTKEY_EDIT_QUIT:
	case hotkey::HOTKEY_EDIT_SET_START_POS:
	case hotkey::HOTKEY_EDIT_NEW_MAP:
	case hotkey::HOTKEY_EDIT_LOAD_MAP:
	case hotkey::HOTKEY_EDIT_FLOOD_FILL:
	case hotkey::HOTKEY_EDIT_FILL_SELECTION:
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

void map_editor::add_undo_action(const map_undo_action &action) {
	undo_stack_.push_back(action);
	if(undo_stack_.size() > undo_limit) {
		undo_stack_.pop_front();
	}
}

void map_editor::undo() {
	if(!undo_stack_.empty()) {
		--num_operations_since_save_;
		map_undo_action action = undo_stack_.back();
		std::vector<gamemap::location> to_invalidate;
		for(std::map<gamemap::location,gamemap::TERRAIN>::const_iterator it = action.undo_terrains().begin();
		    it != action.undo_terrains().end(); ++it) {
			map_.set_terrain(it->first, it->second);
			to_invalidate.push_back(it->first);
		}
		undo_stack_.pop_back();
		redo_stack_.push_back(action);
		if(redo_stack_.size() > undo_limit)
			redo_stack_.pop_front();
		invalidate_all_and_adjacent(to_invalidate);
	}
}

void map_editor::redo() {
	if(!redo_stack_.empty()) {
		++num_operations_since_save_;
		map_undo_action action = redo_stack_.back();
		std::vector<gamemap::location> to_invalidate;
		for(std::map<gamemap::location,gamemap::TERRAIN>::const_iterator it = action.redo_terrains().begin();
		    it != action.redo_terrains().end(); ++it) {
			map_.set_terrain(it->first, it->second);
			to_invalidate.push_back(it->first);
		}
		redo_stack_.pop_back();
		undo_stack_.push_back(action);
		add_undo_action(action);
		invalidate_all_and_adjacent(to_invalidate);
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
		num_operations_since_save_ = undo_limit + 1;
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

void map_editor::left_button_down(const int mousex, const int mousey) {
	const gamemap::location& loc = gui_.minimap_location_on(mousex,mousey);
	const gamemap::location hex = gui_.hex_clicked_on(mousex, mousey);
	if (loc.valid()) {
		gui_.scroll_to_tile(loc.x,loc.y,display::WARP,false);
	}
	else if (key_[SDLK_RSHIFT] || key_[SDLK_LSHIFT]) {
		if (key_[SDLK_RALT] || key_[SDLK_LALT]) {
			// Select/deselect a component.
			std::set<gamemap::location> selected;
			selected = get_component(map_, selected_hex_);
			for (std::set<gamemap::location>::const_iterator it = selected.begin();
				 it != selected.end(); it++) {
				if (add_selection_) {
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
				if (add_selection_) {
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
	else if (draw_terrain_) {
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
					++num_operations_since_save_;
					redo_stack_.clear();
					map_undo_action action;
					std::vector<gamemap::location> to_invalidate;
					for(std::vector<gamemap::location>::const_iterator it = locs.begin();
					    it != locs.end(); ++it) {
						if(palette_.selected_terrain() != map_[it->x][it->y]) {
							to_invalidate.push_back(*it);
							action.add(map_[it->x][it->y], palette_.selected_terrain(), *it);
							map_.set_terrain(*it, palette_.selected_terrain());
						}
					}
					if (!to_invalidate.empty()) {
						add_undo_action(action);
						invalidate_all_and_adjacent(to_invalidate);
					}
				}
			}
		}
	}
}

void map_editor::draw_terrain(const gamemap::TERRAIN terrain,
							  const gamemap::location hex) {
	++num_operations_since_save_;
	redo_stack_.clear();
	const gamemap::TERRAIN current_terrain = map_[hex.x][hex.y];
	add_undo_action(map_undo_action(current_terrain, terrain, hex));
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

void map_editor::right_button_down(const int mousex, const int mousey) {
}
void map_editor::middle_button_down(const int mousex, const int mousey) {
}


bool map_editor::confirm_exit_and_save() {
	if (gui::show_dialog(gui_, NULL, "",
						 "Do you want to exit the map editor?", gui::YES_NO) != 0) {
		return false;
	}
	if (changed_since_save() &&
		gui::show_dialog(gui_, NULL, "",
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
						   const int yloc, const bool context_menu) {
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
	if(res < 0 || res >= items.size())
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
	const int scroll_speed = preferences::scroll_speed();
	int last_brush_size = brush_.selected_brush_size();
	while (abort_ == DONT_ABORT) {
		int mousex, mousey;
		const int mouse_flags = SDL_GetMouseState(&mousex,&mousey);
		const bool l_button_down = mouse_flags & SDL_BUTTON_LMASK;
		const bool r_button_down = mouse_flags & SDL_BUTTON_RMASK;
		const bool m_button_down = mouse_flags & SDL_BUTTON_MMASK;

		const gamemap::location cur_hex = gui_.hex_clicked_on(mousex,mousey);
		if (cur_hex != selected_hex_
			|| last_brush_size != brush_.selected_brush_size()) {
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
		if (r_button_down) {
			right_button_down(mousex, mousey);
		}
		if (m_button_down) {
			middle_button_down(mousex, mousey);
		}

		gui_.draw(false);
		palette_.draw();
		brush_.draw();

		if(tup_.process(mousex,mousey,l_button_down)) {
			palette_.scroll_up();
		}

		if(tdown_.process(mousex,mousey,l_button_down)) {
			palette_.scroll_down();
		}

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
	}
}


}


