/*
  Copyright (C) 2003 by David Whire <davidnwhite@optusnet.com.au>
  Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License.
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY.

  See the COPYING file for more details.
*/
#include "SDL.h"
#include "SDL_keysym.h"

#include "../actions.hpp"
#include "../ai.hpp"
#include "../config.hpp"
#include "../cursor.hpp"
#include "../dialogs.hpp"
#include "../filesystem.hpp"
#include "../font.hpp"
#include "../game_config.hpp"
#include "../gamestatus.hpp"
#include "../image.hpp"
#include "../key.hpp"
#include "../language.hpp"
#include "../mapgen.hpp"
#include "../widgets/menu.hpp"
#include "../pathfind.hpp"
#include "../playlevel.hpp"
#include "../preferences.hpp"
#include "../sdl_utils.hpp"
#include "../widgets/slider.hpp"
#include "../team.hpp"
#include "../unit_types.hpp"
#include "../unit.hpp"
#include "../util.hpp"
#include "../video.hpp"

#include "editor.hpp"

#include <cctype>
#include <iostream>
#include <map>
#include <string>

namespace {
	const unsigned int undo_limit = 100;
	// Milliseconds to sleep in every iteration of the main loop.
	const unsigned int sdl_delay = 20;
	const size_t default_terrain_size = 35;
	const unsigned int minimap_redraw_after_iterations = 3000 / sdl_delay;
}

namespace map_editor {
map_editor::map_editor(display &gui, gamemap &map, config &theme, config &game_config)
	: gui_(gui), map_(map), tup_(gui, "", gui::button::TYPE_PRESS, "uparrow-button"),
	  tdown_(gui, "", gui::button::TYPE_PRESS, "downarrow-button"), abort_(DONT_ABORT),
	  tstart_(0), num_operations_since_save_(0), theme_(theme),
	  game_config_(game_config), draw_terrain_(false),
	  minimap_dirty_(false) {

	terrains_ = map_.get_terrain_precedence();
	terrains_.erase(std::remove_if(terrains_.begin(), terrains_.end(), is_invalid_terrain),
					terrains_.end());
	if(terrains_.empty()) {
		std::cerr << "No terrain found.\n";
		abort_ = ABORT_HARD;
	}
	selected_terrain_ = terrains_[1];

	// Set size specs.
	adjust_sizes(gui_);

	tup_.set_xy(gui.mapx() + size_specs_.button_x, size_specs_.top_button_y);
	tdown_.set_xy(gui.mapx() + size_specs_.button_x, size_specs_.bot_button_y);

	hotkey::add_hotkeys(theme_, true);
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
		if(sym == SDLK_ESCAPE) {
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
			scroll_palette_up();
		}
		if (button == SDL_BUTTON_WHEELDOWN) {
			scroll_palette_down();
		}
		if (button == SDL_BUTTON_RIGHT) {
			selected_hex_ = gui_.hex_clicked_on(mousex, mousey);
			const theme::menu* const m = gui_.get_theme().context_menu();
			show_menu(m->items(), mousex, mousey + 1, true);
		}
		if (button == SDL_BUTTON_LEFT) {
			draw_terrain_ = true;
			int tselect = tile_selected(mousex, mousey, gui_, size_specs_);
			if(tselect >= 0) {
				// Was the click on a terrain item in the palette? If so, set
				// the selected terrain.
				selected_terrain_ = terrains_[tstart_+tselect];
			}
		}
	}
	if (event.type == SDL_MOUSEBUTTONUP) {
		draw_terrain_ = false;
	}
}

std::string map_editor::new_map_dialog(display& disp)
{
	const events::resize_lock prevent_resizing;
	const events::event_context dialog_events_context;

	int map_width(40), map_height(40);
	const int width = 600;
	const int height = 400;
	const int xpos = disp.x()/2 - width/2;
	const int ypos = disp.y()/2 - height/2;
	const int horz_margin = 5;
	const int vertical_margin = 20;

	SDL_Rect dialog_rect = {xpos-10,ypos-10,width+20,height+20};
	surface_restorer restorer(&disp.video(),dialog_rect);

	gui::draw_dialog_frame(xpos,ypos,width,height,disp);

	SDL_Rect title_rect = font::draw_text(NULL,disp.screen_area(),24,font::NORMAL_COLOUR,
					      "Create New Map",0,0);

	const std::string& width_label = string_table["map_width"] + ":";
	const std::string& height_label = string_table["map_height"] + ":";

	SDL_Rect width_rect = font::draw_text(NULL,disp.screen_area(),14,font::NORMAL_COLOUR,width_label,0,0);
	SDL_Rect height_rect = font::draw_text(NULL,disp.screen_area(),14,font::NORMAL_COLOUR,height_label,0,0);

	const int text_right = xpos + horz_margin +
	        maximum<int>(width_rect.w,height_rect.w);

	width_rect.x = text_right - width_rect.w;
	height_rect.x = text_right - height_rect.w;
	
	width_rect.y = ypos + title_rect.h + vertical_margin*2;
	height_rect.y = width_rect.y + width_rect.h + vertical_margin;

	gui::button new_map_button(disp,"Generate New Map With Selected Terrain");
	gui::button random_map_button(disp,"Generate Random Map");
	gui::button random_map_setting_button(disp,"Random Generator Setting");
	gui::button cancel_button(disp,"Cancel");

	new_map_button.set_x(xpos + horz_margin);
	new_map_button.set_y(height_rect.y + height_rect.h + vertical_margin);
	random_map_button.set_x(xpos + horz_margin);
	random_map_button.set_y(ypos + height - random_map_button.height()-14*2-vertical_margin);
	random_map_setting_button.set_x(random_map_button.get_x() + random_map_button.width() + horz_margin);
	random_map_setting_button.set_y(ypos + height - random_map_setting_button.height()-14*2-vertical_margin);
	cancel_button.set_x(xpos + width - cancel_button.width() - horz_margin);
	cancel_button.set_y(ypos + height - cancel_button.height()-14);

	const int right_space = 100;

	const int slider_left = text_right + 10;
	const int slider_right = xpos + width - horz_margin - right_space;
	SDL_Rect slider_rect = { slider_left,width_rect.y,slider_right-slider_left,width_rect.h};

	const int min_width = 20;
	const int max_width = 200;
	const int max_height = 200;
	
	slider_rect.y = width_rect.y;
	gui::slider width_slider(disp,slider_rect);
	width_slider.set_min(min_width);
	width_slider.set_max(max_width);
	width_slider.set_value(map_width);

	slider_rect.y = height_rect.y;
	gui::slider height_slider(disp,slider_rect);
	height_slider.set_min(min_width);
	height_slider.set_max(max_height);
	height_slider.set_value(map_height);

 	const config* const cfg = game_config_.find_child("multiplayer","id","ranmap")->child("generator");
 	util::scoped_ptr<map_generator> generator(NULL);
 	generator.assign(create_map_generator("", cfg));

	for(bool draw = true;; draw = false) {
		int mousex, mousey;
		const int mouse_flags = SDL_GetMouseState(&mousex,&mousey);

		const bool left_button = mouse_flags&SDL_BUTTON_LMASK;

		if(cancel_button.process(mousex,mousey,left_button)) {
			return "";
		}

		if(new_map_button.process(mousex,mousey,left_button)) {
			int i;
			std::stringstream str;
			std::stringstream map_str;
			for (i = 0; i < width_slider.value(); i++) {
				str << selected_terrain_;
			}
			str << "\n";
			for (i = 0; i < height_slider.value(); i++) {
				map_str << str.str();
			}
			return map_str.str();
		}
		if(random_map_setting_button.process(mousex,mousey,left_button)) {
			if (generator.get()->allow_user_config()) {
				generator.get()->user_config(gui_);
			}
		}

		if(random_map_button.process(mousex,mousey,left_button)) {
			const std::string map = generator.get()->create_map(std::vector<std::string>());
			if (map == "") {
				gui::show_dialog(gui_, NULL, "",
						 "Creation failed.", gui::OK_ONLY);
			}
			return map;
		}
		map_width = width_slider.value();
		map_height = height_slider.value();

		gui::draw_dialog_frame(xpos,ypos,width,height,disp);

		width_slider.process();
		height_slider.process();

		width_slider.set_min(min_width);
		height_slider.set_min(min_width);

		events::raise_process_event();
		events::raise_draw_event();

		title_rect = font::draw_text(&disp,disp.screen_area(),24,font::NORMAL_COLOUR,
                       "Create New Map",xpos+(width-title_rect.w)/2,ypos+10);

		font::draw_text(&disp,disp.screen_area(),14,font::NORMAL_COLOUR,width_label,width_rect.x,width_rect.y);
		font::draw_text(&disp,disp.screen_area(),14,font::NORMAL_COLOUR,height_label,height_rect.x,height_rect.y);

		std::stringstream width_str;
		width_str << map_width;
		font::draw_text(&disp,disp.screen_area(),14,font::NORMAL_COLOUR,width_str.str(),
		                slider_right+horz_margin,width_rect.y);

		std::stringstream height_str;
		height_str << map_height;
		font::draw_text(&disp,disp.screen_area(),14,font::NORMAL_COLOUR,height_str.str(),
		                slider_right+horz_margin,height_rect.y);
		
		new_map_button.draw();
		random_map_button.draw();
		random_map_setting_button.draw();
		cancel_button.draw();

		update_rect(xpos,ypos,width,height);

		disp.update_display();
		SDL_Delay(10);
		events::pump();
	}
}

void map_editor::edit_save_map() {
	save_map("", true);
}

void map_editor::edit_quit() {
	set_abort();
}

void map_editor::edit_new_map()
{
	const std::string map = new_map_dialog(gui_);
 	if (map != "") {
		int res = 0;
		if (num_operations_since_save_ != 0) {
			res = gui::show_dialog(gui_, NULL, "",
								   "The modification to the map will be discarded.  Continue?",
								   gui::OK_CANCEL);
		}
		if (res == 0)
			throw new_map_exception(map);
	}
}

void map_editor::edit_save_as() {
	std::string input_name = get_dir(get_dir(get_user_data_dir() + "/editor") + "/maps/");
	int res = 0;
	int overwrite = 0;
	do {
		res = gui::show_dialog(gui_, NULL, "", "Save As ", gui::OK_CANCEL, NULL,NULL,"",&input_name);
					   
		if (res == 0) {
			if (file_exists(input_name)) {
				overwrite = gui::show_dialog(gui_,NULL,"",
							     "Map already exists.  Do you want to overwrite it?",gui::YES_NO);
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
	const int res = gui::show_dialog(gui_, NULL, "", "Which player should start here?",
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
		return true;
	default:
		return false;
	}
}

void map_editor::adjust_sizes(const display &disp) {
	const size_t button_height = 24;
	const size_t button_palette_padding = 8;
	size_specs_.terrain_size = default_terrain_size;
	size_specs_.terrain_padding = 2;
	size_specs_.terrain_space = size_specs_.terrain_size + size_specs_.terrain_padding;
	size_specs_.palette_x = 40;
	size_specs_.button_x = size_specs_.palette_x + size_specs_.terrain_space - 12;
	size_specs_.top_button_y = 190;
	size_specs_.palette_y = size_specs_.top_button_y + button_height +
		button_palette_padding;
	const size_t max_bot_button_y = disp.y() - 60 - button_height;
	size_t space_for_terrains = max_bot_button_y - button_palette_padding -
		size_specs_.palette_y;
	space_for_terrains = space_for_terrains / size_specs_.terrain_space % 2 == 0 ? 
		space_for_terrains : space_for_terrains - size_specs_.terrain_space;
	size_specs_.nterrains = minimum((space_for_terrains / size_specs_.terrain_space) * 2,
									terrains_.size());
	size_specs_.bot_button_y = size_specs_.palette_y +
		(size_specs_.nterrains / 2) * size_specs_.terrain_space + button_palette_padding;
}

void map_editor::toggle_grid() {
	preferences::set_grid(!preferences::grid());
	gui_.set_grid(preferences::grid());
	gui_.invalidate_all();
}

void map_editor::undo() {
	if(!undo_stack_.empty()) {
		--num_operations_since_save_;
		map_undo_action action = undo_stack_.back();
		map_.set_terrain(action.location,action.old_terrain);
		undo_stack_.pop_back();
		redo_stack_.push_back(action);
		if(redo_stack_.size() > undo_limit)
			redo_stack_.pop_front();
		invalidate_adjacent(action.location);
	}
}

void map_editor::redo() {
	if(!redo_stack_.empty()) {
		++num_operations_since_save_;
		map_undo_action action = redo_stack_.back();
		map_.set_terrain(action.location,action.new_terrain);
		redo_stack_.pop_back();
		undo_stack_.push_back(action);
		if(undo_stack_.size() > undo_limit)
			undo_stack_.pop_front();
		invalidate_adjacent(action.location);
	}
}

void map_editor::set_starting_position(const int player, const gamemap::location loc) {
	if(map_.on_board(loc)) {
		map_.set_terrain(loc, gamemap::CASTLE);
		// This operation is currently not undoable, so we need to make sure
		// that save is always asked for after it is performed.
		num_operations_since_save_ = undo_limit + 1;
		map_.set_starting_position(player, loc);
		gui_.invalidate_all();
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
	if (loc.valid()) {
		gui_.scroll_to_tile(loc.x,loc.y,display::WARP,false);
	}
	// If the left mouse button is down and we beforhand have registred
	// a mouse down event, draw terrain at the current location.
	else if (draw_terrain_) {
		const gamemap::location hex = gui_.hex_clicked_on(mousex, mousey);
		if(map_.on_board(hex)) {
			const gamemap::TERRAIN terrain = map_[hex.x][hex.y];
			if(selected_terrain_ != terrain) {
				if(key_[SDLK_RCTRL] || key_[SDLK_LCTRL]) {
					selected_terrain_ = terrain;
				}
				else {
					draw_terrain(selected_terrain_, hex);
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
	undo_stack_.push_back(map_undo_action(current_terrain, terrain, hex));
	if(undo_stack_.size() > undo_limit)
		undo_stack_.pop_front();
	map_.set_terrain(hex, terrain);
	invalidate_adjacent(hex);
}

void map_editor::invalidate_adjacent(const gamemap::location hex) {
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
		gui_.invalidate(locs[i]);
	}
	minimap_dirty_ = true;
}

void map_editor::right_button_down(const int mousex, const int mousey) {
}
void map_editor::middle_button_down(const int mousex, const int mousey) {
}

void map_editor::scroll_palette_down() {
	if(tstart_ + size_specs_.nterrains + 2 <= terrains_.size()) {
		tstart_ += 2;
	}
}

void map_editor::scroll_palette_up() {
	if(tstart_ >= 2) {
		tstart_ -= 2;
	}
}

bool map_editor::confirm_exit_and_save() {
	int exit_res = gui::show_dialog(gui_, NULL, "",
									"Do you want to exit the map editor?", gui::YES_NO);
	if (exit_res != 0) {
		return false;
	}
	if (num_operations_since_save_ != 0) {
		const int save_res = gui::show_dialog(gui_, NULL, "",
											  "Do you want to save before exiting?",
											  gui::YES_NO);
		if(save_res == 0) {
			if (!save_map("", false)) {
				return false;
			}
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
  
void map_editor::main_loop() {
	const double scroll_speed = preferences::scroll_speed();
	unsigned int counter = 0;
	while (abort_ == DONT_ABORT) {
		int mousex, mousey;
		const int mouse_flags = SDL_GetMouseState(&mousex,&mousey);
		const bool l_button_down = mouse_flags & SDL_BUTTON_LMASK;
		const bool r_button_down = mouse_flags & SDL_BUTTON_RMASK;
		const bool m_button_down = mouse_flags & SDL_BUTTON_MMASK;

		const gamemap::location cur_hex = gui_.hex_clicked_on(mousex,mousey);
		gui_.highlight_hex(cur_hex);
		selected_hex_ = cur_hex;

		const theme::menu* const m = gui_.menu_pressed(mousex, mousey,
													   mouse_flags & SDL_BUTTON_LMASK);
		if (m != NULL) {
			const SDL_Rect& menu_loc = m->location(gui_.screen_area());
			const int x = menu_loc.x + 1;
			const int y = menu_loc.y + menu_loc.h + 1;
			show_menu(m->items(), x, y, false);
		}
	
		if(key_[SDLK_UP] || mousey == 0) {
			gui_.scroll(0.0,-scroll_speed);
		}
		if(key_[SDLK_DOWN] || mousey == gui_.y()-1) {
			gui_.scroll(0.0,scroll_speed);
		}
		if(key_[SDLK_LEFT] || mousex == 0) {
			gui_.scroll(-scroll_speed,0.0);
		}
		if(key_[SDLK_RIGHT] || mousex == gui_.x()-1) {
			gui_.scroll(scroll_speed,0.0);
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
		if(drawterrainpalette(gui_, tstart_, selected_terrain_, map_, size_specs_) == false)
			scroll_palette_down();

		if(tup_.process(mousex,mousey,l_button_down)) {
			scroll_palette_up();
		}

		if(tdown_.process(mousex,mousey,l_button_down)) {
			scroll_palette_down();
		}

		if (minimap_dirty_) {
			// If the mini map is dirty, start the counter and redraw after
			// the desired number of iterations.
			++counter;
			if (counter >= minimap_redraw_after_iterations) {
				counter = 0;
				gui_.recalculate_minimap();
				minimap_dirty_ = false;
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

void map_editor::edit_load_map()
{
	const std::string system_path = game_config::path + "/data/maps/";
	
	std::vector<std::string> files;
	get_files_in_dir(system_path,&files);
	files.push_back("Enter Path...");
	files.push_back("Local Map...");
  
	std::string filename;
	const int res = gui::show_dialog(gui_, NULL, "",
					 "Choose map to edit:", gui::OK_CANCEL, &files);
	if(res == int(files.size()-1)) {
		std::vector<std::string> user_files;
		const std::string user_path = get_user_data_dir() + "/editor/maps/";
		get_files_in_dir(user_path,&user_files);
		const int res = gui::show_dialog(gui_, NULL, "",
						 "Choose map to edit:", gui::OK_CANCEL, &user_files);
		if (res < 0) {
			return;
		}
		filename = user_path + user_files[res];
	}
	else if (res == int(files.size() - 2)) {
		filename = get_user_data_dir() + "/editor/maps/";
		const int res = gui::show_dialog(gui_, NULL, "",
										 "Enter map to edit:", gui::OK_CANCEL,
										 NULL, NULL, "", &filename);
		if (res < 0) {
			return;
		}
	}
	else if(res < 0) {
		return;
	}
	else {
		filename = system_path + files[res];
	}
	std::string map;
	bool load_successful = true;
	std::string msg;
	if (!file_exists(filename)) {
		load_successful = false;
		msg = filename + " does not exist.";
	}
	else {
		try {
			map = read_file(filename);
		}
		catch (io_exception e) {
			load_successful = false;
			msg = e.what();
		}
	}
	if (!load_successful) {
		gui::show_dialog(gui_, NULL, "", std::string("Load failed: ") + msg, gui::OK_ONLY);
	}
	else {
		if (num_operations_since_save_ != 0) {
			const int res = gui::show_dialog(gui_, NULL, "",
											 "The modification to the map will be discarded.  Continue?",
											 gui::OK_CANCEL);
			if (res != 0)
				return;
		}
		throw new_map_exception(map);
	}
}

void drawbar(display& disp)
{
	SDL_Surface* const screen = disp.video().getSurface();
	SDL_Rect dst = {disp.mapx(), 0, disp.x() - disp.mapx(), disp.y()};
	SDL_FillRect(screen,&dst,0);
	update_rect(dst);
}

bool drawterrainpalette(display& disp, int start, gamemap::TERRAIN selected, gamemap map,
						size_specs specs)
{
	size_t x = disp.mapx() + specs.palette_x;
	size_t y = specs.palette_y;

	unsigned int starting = start;
	unsigned int ending = starting+specs.nterrains;

	bool status = true;

	SDL_Rect invalid_rect;
	invalid_rect.x = x;
	invalid_rect.y = y;
	invalid_rect.w = 0;

	SDL_Surface* const screen = disp.video().getSurface();

	std::vector<gamemap::TERRAIN> terrains = map.get_terrain_precedence();
	terrains.erase(std::remove_if(terrains.begin(), terrains.end(), is_invalid_terrain),
				   terrains.end());
	if(ending > terrains.size()){
		ending = terrains.size();
		starting = ending - specs.nterrains;
		status = false;
	}

	for(unsigned int counter = starting; counter < ending; counter++){
		const gamemap::TERRAIN terrain = terrains[counter];
		const std::string filename = "terrain/" +
			map.get_terrain_info(terrain).default_image() + ".png";
		scoped_sdl_surface image(image::get_image(filename, image::UNSCALED));
						   
		if(image->w != specs.terrain_size || image->h != specs.terrain_size) {
			image.assign(scale_surface(image, specs.terrain_size, specs.terrain_size));
		}

		if(image == NULL) {
			std::cerr << "image for terrain '" << counter << "' not found\n";
			return status;
		}

		SDL_Rect dstrect;
		dstrect.x = x + (counter % 2 != 0 ?	 0 : specs.terrain_space);
		dstrect.y = y;
		dstrect.w = image->w;
		dstrect.h = image->h;

		SDL_BlitSurface(image, NULL, screen, &dstrect);
		gui::draw_rectangle(dstrect.x, dstrect.y, image->w, image->h,
							terrain == selected ? 0xF000:0 , screen);
	
		if (counter % 2 != 0)
			y += specs.terrain_space;
	}
	invalid_rect.w = specs.terrain_space * 2;
  
	invalid_rect.h = y - invalid_rect.y;
	update_rect(invalid_rect);
	return status;
}

int tile_selected(const unsigned int x, const unsigned int y,
				  const display& disp, const size_specs specs)
{
	for(unsigned int i = 0; i != specs.nterrains; i++) {
		const unsigned int px = disp.mapx() + specs.palette_x +
			(i % 2 != 0 ? 0 : specs.terrain_space);
		const unsigned int py = specs.palette_y + (i / 2) * specs.terrain_space;
		const unsigned int pw = specs.terrain_space;
		const unsigned int ph = specs.terrain_space;

		if(x > px && x < px + pw && y > py && y < py + ph) {
			return i;
		}
	}
	return -1;
}

bool is_invalid_terrain(char c) {
	return c == ' ' || c == '~';
}



}

int main(int argc, char** argv)
{
	game_config::editor = true;

	if(argc > 2) {
		std::cout << "usage: " << argv[0] << " [map-name]" << std::endl;
		return 0;
	}

	CVideo video;

	const font::manager font_manager;
	const preferences::manager prefs_manager;
	const image::manager image_manager;
	std::pair<int, int> desired_resolution = preferences::resolution();
	int video_flags = preferences::fullscreen() ? FULL_SCREEN : 0;
	video.setMode(desired_resolution.first, desired_resolution.second,
				  16, video_flags);

	preproc_map defines_map;
	defines_map["MEDIUM"] = preproc_define();
	defines_map["NORMAL"] = preproc_define();
	config cfg(preprocess_file("data/game.cfg", &defines_map));

	set_language("English");

	std::string filename;
	std::string mapdata;

	if(argc == 2) {
		filename = argv[1];
		try {
			mapdata = read_file(filename);
		}
		catch (io_exception) {
			std::cerr << "Could not read the map file, sorry." << std::endl;
			return 1;
		}
	}
	else {
		filename = "";
	}
	if(mapdata.empty()) {
		for(int i = 0; i != 20; ++i) {
			mapdata = mapdata + "gggggggggggggggggggg\n";
		}
	}

	bool done = false;
	gamestatus status(cfg, 0);
	std::vector<team> teams;
	config* const theme_cfg = cfg.find_child("theme", "name", "editor");
	config dummy_theme;
	if (!theme_cfg) {
		std::cerr << "Editor theme could not be loaded." << std::endl;
		*theme_cfg = dummy_theme;
	}
	std::map<gamemap::location,unit> units;
	while (! done) {
		try {
			gamemap map(cfg, mapdata);

			display gui(units, video, map, status, teams,
				    *theme_cfg, cfg);
			gui.set_grid(preferences::grid());
	
			//Draw the nice background bar
			map_editor::drawbar(gui);

			events::event_context ec;
			map_editor::map_editor editor(gui, map, *theme_cfg, cfg);
			editor.set_file_to_save_as(filename);
			editor.main_loop();
			done = true;
		}
		catch (map_editor::map_editor::new_map_exception &e) {
			std::cerr << "new map " << e.new_map_ << std::endl;
			mapdata = e.new_map_;
			filename = "";
		}
		catch (gamemap::incorrect_format_exception) {
			std::cerr << "The map is not in a correct format, sorry." << std::endl;
			return 1;
		}
	}
	return 0;
}
