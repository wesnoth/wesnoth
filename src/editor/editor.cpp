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
#include "../widgets/menu.hpp"
#include "../pathfind.hpp"
#include "../playlevel.hpp"
#include "../preferences.hpp"
#include "../sdl_utils.hpp"
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
	const double zoom_amount = 5.0;
	// Milliseconds to sleep in every iteration of the main loop.
	const unsigned int sdl_delay = 20;
	const size_t default_terrain_size = 35;
}

namespace map_editor {

editor_event_handler::editor_event_handler(map_editor &editor)
	: editor_(editor) {}

void editor_event_handler::handle_event(const SDL_Event &event) {
	int mousex, mousey;
	SDL_GetMouseState(&mousex,&mousey);
	const SDL_KeyboardEvent keyboard_event = event.key;
	handle_keyboard_event(keyboard_event, mousex, mousey);

	const SDL_MouseButtonEvent mouse_button_event = event.button;
	handle_mouse_button_event(mouse_button_event, mousex, mousey);
}

void editor_event_handler::handle_keyboard_event(const SDL_KeyboardEvent &event,
						 const int mousex, const int mousey) {
	if (event.type == SDL_KEYDOWN) {
		const SDLKey sym = event.keysym.sym;
		if (sym == SDLK_z) {
			editor_.zoom_in();
		}
		if (sym == SDLK_x) {
			editor_.zoom_out();
		}
		if (sym == SDLK_d) {
			editor_.zoom_default();
		}
		if (sym == SDLK_u) {
			editor_.undo();
		}
		if (sym == SDLK_r) {
			editor_.redo();
		}
		if (sym == SDLK_s) {
			editor_.save_map("", true);
		}
		if(sym == SDLK_ESCAPE) {
			editor_.set_abort(ABORT_NORMALLY);
		}
		
		// Check if a number key was pressed.
		for(int num_key = SDLK_1; num_key != SDLK_9; ++num_key) {
			if(sym == num_key) {
	const unsigned int number_pressed = num_key + 1 - SDLK_1;
	editor_.set_starting_position(number_pressed, mousex, mousey);
	break;
			}
		}
	}
}

void editor_event_handler::handle_mouse_button_event(const SDL_MouseButtonEvent &event,
								 const int mousex, const int mousey) {
	if (event.type == SDL_MOUSEBUTTONDOWN) {
		const Uint8 button = event.button;
		if (button == SDL_BUTTON_WHEELUP) {
			editor_.scroll_palette_up();
		}
		if (button == SDL_BUTTON_WHEELDOWN) {
			editor_.scroll_palette_down();
		}
	}
}

map_editor::map_editor(display &gui, gamemap &map)
	: gui_(gui), map_(map), tup_(gui, "", gui::button::TYPE_PRESS, "uparrow-button"),
		tdown_(gui, "", gui::button::TYPE_PRESS, "downarrow-button"), abort_(DONT_ABORT),
		tstart_(0), num_operations_since_save_(0) {

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
}

void map_editor::adjust_sizes(const display &disp) {
	size_specs_.terrain_size = default_terrain_size;
	size_specs_.terrain_padding = 2;
	size_specs_.terrain_space = size_specs_.terrain_size + size_specs_.terrain_padding;
	size_specs_.palette_x = 40;
	size_specs_.button_x = size_specs_.palette_x + size_specs_.terrain_space - 12;
	size_specs_.top_button_y = 200;
	size_specs_.palette_y = size_specs_.top_button_y + 40;
	size_specs_.bot_button_y = disp.y() - 20 - 24;
	size_t space_for_terrains = size_specs_.bot_button_y -
		(size_specs_.top_button_y + 24);
	space_for_terrains = space_for_terrains / size_specs_.terrain_space % 2 == 0 ? 
		space_for_terrains : space_for_terrains - size_specs_.terrain_space;
	size_specs_.nterrains = minimum((space_for_terrains / size_specs_.terrain_space) * 2,
					terrains_.size());
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
		gamemap::location locs[7];
		locs[0] = action.location;
		get_adjacent_tiles(action.location,locs+1);
		for(int i = 0; i != 7; ++i) {
			gui_.draw_tile(locs[i].x,locs[i].y);
		}
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
		gamemap::location locs[7];
		locs[0] = action.location;
		get_adjacent_tiles(action.location,locs+1);
		for(int i = 0; i != 7; ++i) {
			gui_.draw_tile(locs[i].x,locs[i].y);
		}
	}
}

void map_editor::zoom_in() {
	gui_.zoom(zoom_amount);
}

void map_editor::zoom_out() {
	gui_.zoom(-zoom_amount);
}

void map_editor::zoom_default() {
	gui_.default_zoom();
}

void map_editor::set_starting_position(const int player, const int x, const int y) {
	const gamemap::location loc = gui_.hex_clicked_on(x, y);
	if(map_.on_board(loc)) {
		map_.set_terrain(loc, gamemap::CASTLE);
	}
	else {
		std::cerr << "Warning: Set starting position to a hex not on the board." << std::endl;
	}
	map_.set_starting_position(player, loc);
	gui_.invalidate_all();
}

void map_editor::set_abort(const ABORT_MODE abort) {
	abort_ = abort;
}

void map_editor::set_file_to_save_as(const std::string filename) {
	filename_ = filename;
}

void map_editor::left_button_down_(const int mousex, const int mousey) {
	const gamemap::location& loc = gui_.minimap_location_on(mousex,mousey);
	if (loc.valid()) {
		gui_.scroll_to_tile(loc.x,loc.y,display::WARP,false);
	}
	else {
		const gamemap::location hex = gui_.hex_clicked_on(mousex,mousey);
		if(map_.on_board(hex)) {
			const gamemap::TERRAIN terrain = map_[hex.x][hex.y];
			if(selected_terrain_ != terrain) {
	if(key_[SDLK_RCTRL] || key_[SDLK_LCTRL]) {
		selected_terrain_ = terrain;
	} else {
		++num_operations_since_save_;
		undo_stack_.push_back(map_undo_action(terrain,selected_terrain_,hex));
		if(undo_stack_.size() > undo_limit)
			undo_stack_.pop_front();
		map_.set_terrain(hex,selected_terrain_);
	
		gamemap::location locs[7];
		locs[0] = hex;
		get_adjacent_tiles(hex,locs+1);
		for(int i = 0; i != 7; ++i) {
			gui_.draw_tile(locs[i].x,locs[i].y);
		}
		gui_.draw();
		gui_.recalculate_minimap();
	}
			}
		} else {
			int tselect = tile_selected(mousex, mousey, gui_, size_specs_);
			if(tselect >= 0)
	selected_terrain_ = terrains_[tstart_+tselect];
		}
	}
}

void map_editor::right_button_down_(const int mousex, const int mousey) {
}
void map_editor::middle_button_down_(const int mousex, const int mousey) {
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

bool map_editor::confirm_exit_and_save_() {
	int exitRes = gui::show_dialog(gui_, NULL, "Exit?",
				 "Do you want to exit the map editor?", gui::YES_NO);
	if (exitRes != 0) {
		return false;
	}
	if (num_operations_since_save_ > 0) {
		int saveRes = gui::show_dialog(gui_, NULL, "Save?",
					 "Do you want to save before exiting?", gui::YES_NO);
		if(saveRes == 0) {
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
		write_file(filename,map_.write());
		num_operations_since_save_ = 0;
		if (display_confirmation) {
			gui::show_dialog(gui_, NULL, "Saved", "Map saved.", gui::OK_ONLY);
		}
	}
	catch (io_exception) {
		gui::show_dialog(gui_, NULL, "Save Failed", "Could not save the map.", gui::OK_ONLY);
		return false;
	}
	return true;
}

void map_editor::main_loop() {
	events::event_context ec;
	editor_event_handler event_handler(*this);
	const double scroll_speed = preferences::scroll_speed();

	while (abort_ == DONT_ABORT) {
		int mousex, mousey;
		const int mouse_flags = SDL_GetMouseState(&mousex,&mousey);
		const bool left_button_down = mouse_flags & SDL_BUTTON_LMASK;
		const bool right_button_down = mouse_flags & SDL_BUTTON_RMASK;
		const bool middle_button_down = mouse_flags & SDL_BUTTON_MMASK;

		const gamemap::location cur_hex = gui_.hex_clicked_on(mousex,mousey);
		gui_.highlight_hex(cur_hex);

	const theme::menu* const m = gui_.menu_pressed(mousex,mousey,mouse_flags&SDL_BUTTON_LMASK);

	if(m != NULL) {
		std::cerr << "menu pressed\n";
		const SDL_Rect& menu_loc = m->location(gui_.screen_area());
		static const std::string style = "menu2";
		const int res = gui::show_dialog(gui_,NULL,"","",
						 gui::MESSAGE,&m->items(),
						 NULL,"",NULL,NULL,NULL,
						 menu_loc.x+1,menu_loc.y,
						 &style);
		if(0 <= res && res < m->items().size()) {
			if(m->items()[res] == "save") {
				try {
					write_file(filename_,map_.write());
				} catch(io_exception&) {
					gui::show_dialog(gui_,NULL,"",
							 "Save failed",gui::MESSAGE);
				}
			}
			else if(m->items()[res] == "quit")
				break;
		}
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
		
		if (left_button_down) {
			left_button_down_(mousex, mousey);
		}
		if (right_button_down) {
			right_button_down_(mousex, mousey);
		}
		if (middle_button_down) {
			middle_button_down_(mousex, mousey);
		}

		gui_.draw(false);
		if(drawterrainpalette(gui_, tstart_, selected_terrain_, map_, size_specs_) == false)
			scroll_palette_down();

		if(tup_.process(mousex,mousey,left_button_down)) {
			scroll_palette_up();
		}

		if(tdown_.process(mousex,mousey,left_button_down)) {
			scroll_palette_down();
		}

		gui_.update_display();
		SDL_Delay(sdl_delay);
		events::pump();
		if (abort_ == ABORT_NORMALLY) {
			if (!confirm_exit_and_save_()) {
	set_abort(DONT_ABORT);
			}
		}
	}
}


std::string get_filename_from_dialog(CVideo &video, config &cfg) {
	const std::string path = "data/maps/";
	
	display::unit_map u_map;
	config dummy_cfg("");
	
	config dummy_theme;
	display disp(u_map, video, gamemap(dummy_cfg,"1"), gamestatus(dummy_cfg,0),
				 std::vector<team>(), dummy_theme, cfg);
	
	std::vector<std::string> files;
	get_files_in_dir(path,&files);
	
	files.push_back("New Map...");
	
	const int res = gui::show_dialog(disp, NULL, "",
					 "Choose map to edit:", gui::OK_CANCEL, &files);
	if(res < 0) {
		return "";
	}
	std::string filename;
	if(res == int(files.size()-1)) {
		filename = "new-map";
		gui::show_dialog(disp, NULL, "", "Create new map",
				 gui::OK_ONLY, NULL, NULL, "", &filename);
		if (filename == "") {
			// If no name was given, return the empty filename; don't add
			// the path.
			return filename;
		}
	} else {
		filename = files[res];
	}
	filename = path + filename;
	return filename;
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
		dstrect.x = x + (counter % 2 != 0 ?	0 : specs.terrain_space);
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
		const unsigned int px = disp.mapx() + specs.palette_x + (i % 2 != 0 ? 0 : specs.terrain_space);
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
	std::pair<int, int> desiredResolution = preferences::resolution();
	video.setMode(desiredResolution.first,desiredResolution.second,16,0);

	preproc_map defines_map;
	defines_map["MEDIUM"] = preproc_define();
	defines_map["NORMAL"] = preproc_define();
	config cfg(preprocess_file("data/game.cfg",&defines_map));

	set_language("English");

	std::string filename;

	if(argc == 1) {
		filename = map_editor::get_filename_from_dialog(video, cfg);
		if (filename == "") {
			return 0;
		}
	} else if(argc == 2) {
		filename = argv[1];
	}
	
	std::string mapdata;
	try {
		mapdata = read_file(filename);
	}
	catch (io_exception) {
		std::cerr << "Could not read the map file, sorry." << std::endl;
		return 1;
	}
	if(mapdata.empty()) {
		for(int i = 0; i != 30; ++i) {
			mapdata = mapdata + "gggggggggggggggggggggggggggggggggggggg\n";
		}
	}
	try {
		gamemap map(cfg, mapdata);
		gamestatus status(cfg, 0);
		std::vector<team> teams;
		
		const config* const theme_cfg = cfg.find_child("theme", "name", "editor");
		config dummy_theme;
		std::map<gamemap::location,unit> units;
		display gui(units, video, map, status, teams,
		theme_cfg ? *theme_cfg : dummy_theme, cfg);
		
		//Draw the nice background bar
		map_editor::drawbar(gui);
		map_editor::map_editor editor(gui, map);
		editor.set_file_to_save_as(filename);
		editor.main_loop();
	}
	catch (gamemap::incorrect_format_exception) {
		std::cerr << "The map is not in a correct format, sorry." << std::endl;
		return 1;
	}
	return 0;
}
