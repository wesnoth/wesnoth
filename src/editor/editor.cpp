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

#include "../actions.hpp"
#include "../ai.hpp"
#include "../config.hpp"
#include "../cursor.hpp"
#include "../dialogs.hpp"
#include "../display.hpp"
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
#include "../team.hpp"
#include "../unit_types.hpp"
#include "../unit.hpp"
#include "../video.hpp"
#include "../events.hpp"

#include <cctype>
#include <iostream>
#include <map>
#include <string>

namespace {
	const size_t nterrains = 24;
	const size_t terrain_size = 35;
	const size_t terrain_padding = 2;
	const size_t terrain_space = terrain_size + terrain_padding;
	const size_t button_x = 50;
	const size_t top_button_y = 200;
	const size_t palette_x = 40;
	const size_t palette_y = top_button_y + 40;
	const size_t bot_button_y = palette_y + terrain_space*nterrains/2;

	bool is_invalid_terrain(char c) { return c == ' ' || c == '~'; }

	const int undo_limit = 20;
}

struct map_undo_action {
	map_undo_action(const gamemap::TERRAIN& old_tr,
			const gamemap::TERRAIN& new_tr,
			const gamemap::location& lc)
		: old_terrain(old_tr), new_terrain(new_tr), location(lc) {}
	gamemap::TERRAIN old_terrain;
	gamemap::TERRAIN new_terrain;
	gamemap::location location;
};
typedef std::deque<map_undo_action> map_undo_list;

void drawbar(display& disp);
bool drawterrainpalette(display& disp, int start, gamemap::TERRAIN selected, gamemap map);
int tileselected(int x, int y, display& disp);

// A handler that intercepts key events.
class editor_key_handler : public events::handler {
public:
	editor_key_handler(display &gui, map_undo_list &undo_stack, map_undo_list &redo_stack, gamemap &map);
	virtual void handle_event(const SDL_Event &event);
private:
	map_undo_list &undo_stack_, &redo_stack_;
	display &gui_;
	gamemap &map_;
	const double zoom_amount_;
};

editor_key_handler::editor_key_handler(display &gui, map_undo_list &undo_stack, map_undo_list &redo_stack, gamemap &map)
	: gui_(gui), undo_stack_(undo_stack), redo_stack_(redo_stack), map_(map), zoom_amount_(5.0) {}

void editor_key_handler::handle_event(const SDL_Event &event) {
	const SDL_KeyboardEvent keyboard_event = event.key;
	if (keyboard_event.type == SDL_KEYDOWN) {
		const SDLKey sym = keyboard_event.keysym.sym;
		if(sym == SDLK_z)
			gui_.zoom(zoom_amount_);

		if(sym == SDLK_x)
			gui_.zoom(-zoom_amount_);

		if(sym == SDLK_d)
			gui_.default_zoom();

		if(sym == SDLK_u) {
			if(!undo_stack_.empty()) {
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
		
		if(sym == SDLK_r) {
			if(!redo_stack_.empty()) {
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
		
		int mousex, mousey;
		const int mouse_flags = SDL_GetMouseState(&mousex,&mousey);
		const gamemap::location cur_hex = gui_.hex_clicked_on(mousex,mousey);
		for(int num_key = SDLK_1; num_key != SDLK_9; ++num_key) {
			if(sym == num_key) {
				if(map_.on_board(cur_hex)) {
					map_.set_terrain(cur_hex,gamemap::CASTLE);
				}
				map_.set_starting_position(num_key+1-SDLK_1,cur_hex);
				gui_.invalidate_all();
				break;
			}
		}
	}
}

int main(int argc, char** argv)
{
	game_config::editor = true;

	if(argc > 2) {
		std::cout << "usage: " << argv[0] << " map-name\n";
		return 0;
	}

	const double scroll_speed = preferences::scroll_speed();

	CVideo video;

	video.setMode(1024,768,16,0);

	const font::manager font_manager;
	const preferences::manager prefs_manager;
	const image::manager image_manager;

	preproc_map defines_map;
	defines_map["MEDIUM"] = preproc_define();
	defines_map["NORMAL"] = preproc_define();
	config cfg(preprocess_file("data/game.cfg",&defines_map));

	set_language("English");

	std::string filename;

	if(argc == 1) {

		const std::string path = "data/maps/";

		display::unit_map u_map;
		config dummy_cfg("");

		config dummy_theme;
		display disp(u_map,video,gamemap(dummy_cfg,"1"),gamestatus(dummy_cfg,0),
		             std::vector<team>(), dummy_theme, cfg);

		std::vector<std::string> files;
		get_files_in_dir(path,&files);

		files.push_back("New Map...");

		const int res = gui::show_dialog(disp,NULL,"","Choose map to edit:",gui::OK_CANCEL,&files);
		if(res < 0) {
			return 0;
		}

		if(res == int(files.size()-1)) {
			filename = "new-map";
			gui::show_dialog(disp,NULL,"","Create new map",gui::OK_ONLY,NULL,NULL,"",&filename);
			if(filename == "")
				return 0;
		} else {
			filename = files[res];
		}

		filename = path + filename;

	} else if(argc == 2) {
		filename = argv[1];
	}

	std::cout << "a\n";
	std::string mapdata = read_file(filename);
	if(mapdata.empty()) {
		for(int i = 0; i != 30; ++i) {
			mapdata = mapdata + "gggggggggggggggggggggggggggggggggggggg\n";
		}
	}
	
	std::cout << "b\n";
	gamemap map(cfg,mapdata);
	
	CKey key;
	gamestatus status(cfg,0);
	std::vector<team> teams;

	const config* const theme_cfg = cfg.find_child("theme","name",preferences::theme());
	config dummy_theme;
	std::map<gamemap::location,unit> units;
	display gui(units,video,map,status,teams,theme_cfg ? *theme_cfg : dummy_theme, cfg);

	std::vector<std::string> terrain_names;
	std::vector<gamemap::TERRAIN> terrains = map.get_terrain_precedence();
	terrains.erase(std::remove_if(terrains.begin(),terrains.end(),is_invalid_terrain),terrains.end());
	if(terrains.empty()) {
		std::cerr << "No terrain found\n";
		return 0;
	}

	for(std::vector<gamemap::TERRAIN>::const_iterator t = terrains.begin(); t != terrains.end(); ++t) {
		terrain_names.push_back(map.terrain_name(*t));
	}

	gui::button tup(gui, "", gui::button::TYPE_PRESS,"uparrow-button");
	gui::button tdown(gui, "", gui::button::TYPE_PRESS,"downarrow-button");
	tup.set_xy(gui.mapx() + button_x, top_button_y);
	tdown.set_xy(gui.mapx() + button_x, bot_button_y);

	gamemap::TERRAIN selected_terrain = terrains[1];
	int tstart = 0;

	//Draw the nice background bar
	drawbar(gui);

	map_undo_list undo_stack;
	map_undo_list redo_stack;

	events::event_context ec;
	editor_key_handler key_handler(gui, undo_stack, redo_stack, map);
	std::cerr << "starting for(;;)\n";
	for(;;) {
		if(key[SDLK_ESCAPE])
			break;

		int mousex, mousey;
		const int mouse_flags = SDL_GetMouseState(&mousex,&mousey);
		const bool new_left_button = mouse_flags & SDL_BUTTON_LMASK;
		const bool new_right_button = mouse_flags & SDL_BUTTON_RMASK;

		if(key[SDLK_UP] || mousey == 0)
			gui.scroll(0.0,-scroll_speed);

		if(key[SDLK_DOWN] || mousey == gui.y()-1)
			gui.scroll(0.0,scroll_speed);

		if(key[SDLK_LEFT] || mousex == 0)
			gui.scroll(-scroll_speed,0.0);

		if(key[SDLK_RIGHT] || mousex == gui.x()-1)
			gui.scroll(scroll_speed,0.0);

		const gamemap::location cur_hex = gui.hex_clicked_on(mousex,mousey);
		gui.highlight_hex(cur_hex);
		if(new_left_button) {

			const gamemap::location& loc = gui.minimap_location_on(mousex,mousey);
			if (loc.valid()) {
				gui.scroll_to_tile(loc.x,loc.y,display::WARP,false);
				continue;
			}

			const gamemap::location hex = gui.hex_clicked_on(mousex,mousey);
			if(map.on_board(hex)) {
				const gamemap::TERRAIN terrain = map[hex.x][hex.y];
				if(selected_terrain != terrain) {
					if(key[SDLK_RCTRL] || key[SDLK_LCTRL]) {
						selected_terrain = terrain;
					} else {
						undo_stack.push_back(map_undo_action(terrain,selected_terrain,hex));
						if(undo_stack.size() > undo_limit)
							undo_stack.pop_front();
						map.set_terrain(hex,selected_terrain);

						gamemap::location locs[7];
						locs[0] = hex;
						get_adjacent_tiles(hex,locs+1);
						for(int i = 0; i != 7; ++i) {
							gui.draw_tile(locs[i].x,locs[i].y);
						}

						gui.draw();
						gui.recalculate_minimap();
					}
				}
			}else{
				int tselect = tileselected(mousex,mousey,gui);
				if(tselect >= 0)
					selected_terrain = terrains[tstart+tselect];
			}
		}


		gui.draw(false);
		if(drawterrainpalette(gui, tstart, selected_terrain, map)==false)
			tstart += 2;

		if(tup.process(mousex,mousey,new_left_button)) {
			tstart -= 2;
			if(tstart<0)
				tstart=0;
		}

		if(tdown.process(mousex,mousey,new_left_button)) {
			tstart += 2;
			if(tstart+nterrains>terrains.size())
				tstart-=2;
		}

		gui.update_display();
		SDL_Delay(20);
		events::pump();
	}

	int res = gui::show_dialog(gui,NULL,"Save?","Do you want to save changes?",
	                           gui::YES_NO);
	if(res == 0) {
		write_file(filename,map.write());
	}

	return 0;
}

void drawbar(display& disp)
{
	SDL_Surface* const screen = disp.video().getSurface();
	SDL_Rect dst = {disp.mapx(),0,disp.x()-disp.mapx(),disp.y()};
	SDL_FillRect(screen,&dst,0);
	update_rect(dst);
}

bool drawterrainpalette(display& disp, int start, gamemap::TERRAIN selected, gamemap map)
{
	int x = disp.mapx() + palette_x;
	int y = palette_y;

	int starting = start;
	int ending = starting+nterrains;

	bool status = true;

	SDL_Rect invalid_rect;
	invalid_rect.x = x;
	invalid_rect.y = y;
	invalid_rect.w = 0;

	SDL_Surface* const screen = disp.video().getSurface();

	std::vector<gamemap::TERRAIN> terrains = map.get_terrain_precedence();
	terrains.erase(std::remove_if(terrains.begin(),terrains.end(),is_invalid_terrain),terrains.end());
	if(ending > terrains.size()){
		ending = terrains.size();
		starting = ending - nterrains;
		status = false;
	}

	for(int counter = starting; counter < ending; counter++){
		const gamemap::TERRAIN terrain = terrains[counter];
		scoped_sdl_surface image(image::get_image("terrain/" + map.get_terrain_info(terrain).default_image() + ".png",image::UNSCALED));
		if(image->w != terrain_size || image->h != terrain_size) {
			image.assign(scale_surface(image,terrain_size,terrain_size));
		}

		if(image == NULL) {
			std::cerr << "image for terrain '" << counter << "' not found\n";
			return status;
		}

		SDL_Rect dstrect;
		dstrect.x = x + (counter % 2 != 0 ?  0 : terrain_space);
		dstrect.y = y;
		dstrect.w = image->w;
		dstrect.h = image->h;

		SDL_BlitSurface(image,NULL,screen,&dstrect);
		gui::draw_rectangle(dstrect.x,dstrect.y,image->w,image->h,
		                    terrain == selected?0xF000:0,screen);
		
		if (counter % 2 != 0)
			y += terrain_space;
	}
	invalid_rect.w = terrain_space * 2;

	invalid_rect.h = y - invalid_rect.y;
	update_rect(invalid_rect);
	return status;
}

int tileselected(int x, int y, display& disp)
{
	for(int i = 0; i != nterrains; i++) {
		const int px = disp.mapx() + palette_x + (i % 2 != 0 ? 0 : terrain_space);
		const int py = palette_y + (i/2)*terrain_space;
		const int pw = terrain_space;
		const int ph = terrain_space;

		if(x>px && x<px+pw && y>py && y<py+ph) {
			return i;
		}
	}

	return -1;
}

