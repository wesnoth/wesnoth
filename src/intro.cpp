/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "events.hpp"
#include "font.hpp"
#include "game_config.hpp"
#include "image.hpp"
#include "intro.hpp"
#include "key.hpp"
#include "language.hpp"
#include "show_dialog.hpp"
#include "sound.hpp"
#include "util.hpp"
#include "video.hpp"
#include "widgets/button.hpp"
#include "game_events.hpp"

#include <cstdlib>
#include <sstream>
#include <vector>

namespace {
	const int min_room_at_bottom = 150;
}

bool show_intro_part(display& screen, const config& part);

void show_intro(display& screen, const config& data)
{
	std::cerr << "showing intro sequence...\n";
	const events::resize_lock stop_resizing;
	const events::event_context context;

	const std::string& music = data["music"];
	if(music != "") {
		sound::play_music(music);
	}

	bool showing = true;
	
	// it seems the best way to allow "if" tags in the story 
	// is to use a little nested loop, like so.
	for (config::all_children_iterator i = data.ordered_begin();
		 showing && i != data.ordered_end(); ++i) {

		const std::pair<const std::string*, const config*> item = *i;
		if (*item.first == "if") {
			const std::string type = game_events::conditional_passed(
							NULL, *item.second) ? "then":"else";
				
			const config::child_list& thens = (*item.second).get_children(type);
			for (config::child_list::const_iterator t = thens.begin();
				 showing && t != thens.end(); ++t) {

				const config::child_list& parts = (**t).get_children("part");
				for (config::child_list::const_iterator p = parts.begin();
					 showing && p != parts.end(); ++p) {
					 
					showing = show_intro_part(screen, **p);
				}
			}
		}
		else if (*item.first == "part") {
			showing = show_intro_part(screen, *item.second);
		}
	}
	std::cerr << "intro sequence finished...\n";
			
}

bool show_intro_part(display& screen, const config& part)
{
	std::cerr << "showing intro part\n";

	CKey key;

	gui::button next_button(screen,string_table["next_button"] + ">>>");
	gui::button skip_button(screen,string_table["skip_button"]);

	gui::draw_solid_tinted_rectangle(0,0,screen.x()-1,screen.y()-1,
			0,0,0,1.0,screen.video().getSurface());
	const std::string& image_name = part["image"];

	surface image(NULL);
	if(image_name.empty() == false) {
		image.assign(image::get_image(image_name,image::UNSCALED));
	}

	int textx = 200;
	int texty = 400;

	if(image != NULL) {
		SDL_Rect dstrect;
		dstrect.x = screen.x()/2 - image->w/2;
		dstrect.y = screen.y()/2 - image->h/2;
		dstrect.w = image->w;
		dstrect.h = image->h;

		if(dstrect.y + dstrect.h > screen.y() - min_room_at_bottom) {
			dstrect.y = maximum<int>(0,screen.y() - dstrect.h - min_room_at_bottom);
		}

		SDL_BlitSurface(image,NULL,screen.video().getSurface(),&dstrect);

		textx = dstrect.x;
		texty = dstrect.y + dstrect.h + 10;

		next_button.set_location(dstrect.x+dstrect.w-40,dstrect.y+dstrect.h+20);
		skip_button.set_location(dstrect.x+dstrect.w-40,dstrect.y+dstrect.h+70);
	} else {
		next_button.set_location(screen.x()-200,screen.y()-150);
		skip_button.set_location(screen.x()-200,screen.y()-100);
	}


	next_button.draw();
	skip_button.draw();
	update_whole_screen();
	screen.video().flip();

	const std::string& id = part["id"];
	const std::string& lang_story = string_table[id];
	const std::string& story = lang_story.empty() ? part["story"] : lang_story;
	const std::vector<std::string> story_chars = split_utf8_string(story);

	std::cerr << story << std::endl;

	std::vector<std::string>::const_iterator j = story_chars.begin();

	bool skip = false, last_key = true;

	int xpos = textx, ypos = texty;
	
	//the maximum position that text can reach before wrapping
	const int max_xpos = next_button.location().x - 10;
	size_t height = 0;
	//std::string buf;
	
	for(;;) {
		if(j != story_chars.end()) {
			//unsigned char c = *j;
			if(*j == " ") {
				//we're at a space, so find the next space or end-of-text,
				//to find out if the next word will fit, or if it has to be wrapped
				std::vector<std::string>::const_iterator end_word = std::find(j+1,story_chars.end()," ");

				std::string word;
				for(std::vector<std::string>::const_iterator k = j+1;
						k != end_word; ++k) {
					word += *k;
				}
				const SDL_Rect rect = font::draw_text(NULL,screen.screen_area(),
						16,font::NORMAL_COLOUR,
						word,xpos,ypos,NULL,
						false,font::NO_MARKUP);

				if(xpos + rect.w >= max_xpos) {
					xpos = textx;
					ypos += height;
					++j;
					continue;
				}
			}

			// output the character
			const SDL_Rect rect = font::draw_text(&screen,
					screen.screen_area(),16,
					font::NORMAL_COLOUR,*j,xpos,ypos,
					NULL,false,font::NO_MARKUP);

			if(rect.h > height)
				height = rect.h;
			xpos += rect.w; 
			update_rect(rect);

			++j;
			if(j == story_chars.end())
				skip = true;

		}

		int mousex, mousey;
		const int mouse_flags = SDL_GetMouseState(&mousex,&mousey);
		const bool left_button = mouse_flags&SDL_BUTTON_LMASK;

		const bool keydown = key[SDLK_SPACE] || key[SDLK_RETURN];

		if(keydown && !last_key ||
				next_button.process(mousex,mousey,left_button)) {
			if(skip == true)
				break;
			else
				skip = true;
		}

		last_key = keydown;

		if(key[SDLK_ESCAPE] ||
				skip_button.process(mousex,mousey,left_button))
			return false;

		events::pump();
		events::raise_process_event();
		events::raise_draw_event();
		screen.video().flip();

		if(!skip || j == story_chars.end())
			SDL_Delay(20);
	}
	
	gui::draw_solid_tinted_rectangle(0,0,screen.x()-1,screen.y()-1,0,0,0,1.0,
                                     screen.video().getSurface());

	return true;
}

void display_map_scene(display& screen, const std::string& scenario,
		const config::child_list& dots, const std::string& image_file) {

	const surface image(image::get_image(image_file,image::UNSCALED));
	const surface dot_image(image::get_image(game_config::dot_image,image::UNSCALED));
	const surface cross_image(image::get_image(game_config::cross_image,image::UNSCALED));
	if(image == NULL || dot_image == NULL || cross_image == NULL) {
		std::cerr << "could not find map image: '" << image_file << "': " << (image == NULL ? "failed" : "ok") << "\n"
			<< "'" << game_config::dot_image << "': " << (dot_image == NULL ? "failed" : "ok") << "\n"
			<< "'" << game_config::cross_image << "': " << (cross_image == NULL ? "failed" : "ok") << "\n";
		return;
	}

	SDL_Rect dstrect;
	dstrect.x = screen.x()/2 - image->w/2;
	dstrect.y = screen.y()/2 - image->h/2;
	dstrect.w = image->w;
	dstrect.h = image->h;

	if(dstrect.y + dstrect.h > screen.y() - min_room_at_bottom) {
		dstrect.y = maximum<int>(0,screen.y() - dstrect.h - min_room_at_bottom);
	}

	SDL_BlitSurface(image,NULL,screen.video().getSurface(),&dstrect);
	update_whole_screen();

	screen.video().flip();

	CKey key;

	for(std::vector<config*>::const_iterator d = dots.begin(); d != dots.end(); ++d){
		const std::string& xloc = (**d)["x"];
		const std::string& yloc = (**d)["y"];
		const int x = atoi(xloc.c_str());
		const int y = atoi(yloc.c_str());
		if(x < 0 || x >= image->w || y < 0 || y >= image->w)
			continue;

		surface img = dot_image;
		if((**d)["type"] == "cross") {
			img = cross_image;
		}

		int xdot = x - img->w/2;
		int ydot = y - img->h/2;

		if(xdot < 0)
			xdot = 0;

		if(ydot < 0)
			ydot = 0;

		SDL_Rect dot_rect;
		dot_rect.x = xdot + dstrect.x;
		dot_rect.y = ydot + dstrect.y;
		dot_rect.w = img->w;
		dot_rect.h = img->h;

		SDL_BlitSurface(img,NULL,screen.video().getSurface(),&dot_rect);

		update_rect(dot_rect);

		for(int i = 0; i != 50; ++i) {
			if(key[SDLK_ESCAPE]) {
				std::cerr << "escape pressed..\n";
				break;
			}

			SDL_Delay(10);

			events::pump();

			int a, b;
			const int mouse_flags = SDL_GetMouseState(&a,&b);
			if(key[SDLK_RETURN] || key[SDLK_SPACE] || mouse_flags) {
				std::cerr << "key pressed..\n";
				break;
			}

			screen.video().flip();
		}

		if(key[SDLK_ESCAPE]) {
			std::cerr << "escape pressed..\n";
			break;
		}
	}

	if(!key[SDLK_ESCAPE]) {
		for(int i = 0; i != 50; ++i) {
			SDL_Delay(10);
			screen.video().flip();
		}
	}

	static const SDL_Rect area = {0,0,screen.x(),screen.y()};
	const SDL_Rect scenario_size =
	      font::draw_text(NULL,area,24,font::NORMAL_COLOUR,scenario,0,0);
	update_rect(font::draw_text(&screen,area,24,font::NORMAL_COLOUR,scenario,
		                        dstrect.x,dstrect.y - scenario_size.h - 4));

	screen.video().flip();

	bool last_state = true;
	for(;;) {
		int a, b;
		const int mouse_flags = SDL_GetMouseState(&a,&b);

		const bool new_state = mouse_flags || key[SDLK_ESCAPE] ||
		           key[SDLK_RETURN] || key[SDLK_SPACE];

		if(new_state && !last_state) {
			std::cerr << "key pressed..\n";
			break;
		}

		last_state = new_state;

		SDL_Delay(20);
		events::pump();
		screen.video().flip();
	}

	//clear the screen
	gui::draw_solid_tinted_rectangle(0,0,screen.x()-1,screen.y()-1,0,0,0,1.0,
                                     screen.video().getSurface());
}

void show_map_scene_cfg(display& screen, const std::string& scenario,
		const config& cfg) {
	config::all_children_iterator i = cfg.ordered_begin();
	std::pair<const std::string*, const config*> item = *i;

	if(*item.first == "if") {
		const std::string type = game_events::conditional_passed(
				NULL, *item.second) ? "then":"else";
		const config* const thens = (*item.second).child(type);
		if(thens == NULL) {
			std::cerr << "no map scene this way...\n";
			return;
		}
		const config& selection = *thens;
		show_map_scene_cfg(screen, scenario, selection);
	}else{
		const config::child_list& dots = cfg.get_children("dot");
		const std::string& image_file = cfg["image"];
		display_map_scene(screen, scenario, dots, image_file);
	}
}

void show_map_scene(display& screen, config& data)
{
	std::cerr << "showing map scene...\n";
	//stop the screen being resized while we're in this function
	const events::resize_lock stop_resizing;
	const events::event_context context;

	//clear the screen
	gui::draw_solid_tinted_rectangle(0,0,screen.x()-1,screen.y()-1,0,0,0,1.0,
                                     screen.video().getSurface());


	const config* const cfg_item = data.child("bigmap");
	if(cfg_item == NULL) {
		std::cerr << "no map scene...\n";
		return;
	}

	const std::string& id = data.values["id"];
	const std::string& scenario_name = string_table[id];

	const std::string& scenario = scenario_name.empty() ? data.values["name"] :
	                                                      scenario_name;

	const config& cfg = *cfg_item;

	show_map_scene_cfg(screen, scenario, cfg);
}
