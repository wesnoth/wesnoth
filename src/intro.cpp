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

#include <cstdlib>
#include <sstream>
#include <vector>

namespace {
	const int min_room_at_bottom = 150;
}

void show_intro(display& screen, const config& data)
{
	std::cerr << "showing intro sequence...\n";

	//stop the screen being resized while we're in this function
	const events::resize_lock stop_resizing;

	CKey key;

	gui::button next_button(screen,string_table["next_button"] + ">>>");
	gui::button skip_button(screen,string_table["skip_button"]);

	next_button.set_x(screen.x()-200);
	next_button.set_y(screen.y()-150);
	skip_button.set_x(screen.x()-200);
	skip_button.set_y(screen.y()-100);

	const std::string& music = data["music"];
	if(music != "") {
		sound::play_music(music);
	}

	const config::child_list& parts = data.get_children("part");

	for(config::child_list::const_iterator i = parts.begin(); i != parts.end(); ++i){
		gui::draw_solid_tinted_rectangle(0,0,screen.x()-1,screen.y()-1,
		                                 0,0,0,1.0,screen.video().getSurface());
		const std::string& image_name = (**i)["image"];
		
		scoped_sdl_surface image(NULL);
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

			next_button.set_x(dstrect.x+dstrect.w-40);
			next_button.set_y(dstrect.y+dstrect.h+20);
			skip_button.set_x(dstrect.x+dstrect.w-40);
			skip_button.set_y(dstrect.y+dstrect.h+70);
		}

		next_button.draw();
		skip_button.draw();
		update_whole_screen();
		screen.video().flip();

		const std::string& id = (**i)["id"];
		const std::string& lang_story = string_table[id];
		const std::string& story = lang_story.empty() ? (**i)["story"] : lang_story;

		std::string::const_iterator j = story.begin();

		bool skip = false, last_key = true;
		
		int xpos = textx, ypos = texty;

		//the maximum position that text can reach before wrapping
		const int max_xpos = next_button.location().x - 10;
		size_t height = 0;
		std::string buf;
		for(;;) {
			if(j != story.end()) {
				unsigned char c = *j;
				if(c == ' ') {
					//we're at a space, so find the next space or end-of-text,
					//to find out if the next word will fit, or if it has to be wrapped
					std::string::const_iterator end_word = std::find(j+1,story.end(),' ');
					const SDL_Rect rect = font::draw_text(NULL,screen.screen_area(),16,font::NORMAL_COLOUR,
						                                  std::string(j,end_word),xpos,ypos,NULL,
														  false,font::NO_MARKUP);

					if(xpos + rect.w >= max_xpos) {
						xpos = textx;
						ypos += height;
						++j;
						continue;
					}
				}

				//how many bytes does the current utf8 character require?
				int bytes = 1;

				if(charset() == CHARSET_UTF8) {
					if(j[bytes-1] >= 0xC0U) 
						bytes++;
					if(j[bytes-1] >= 0xE0U)
						bytes++;
					if(j[bytes-1] >= 0xF0U)
						bytes++;
					if(j[bytes-1] >= 0xF8U)
						bytes++;
					if(j[bytes-1] >= 0xFCU)
						bytes++;
				}

				//copy the character
				buf.resize(bytes);
				for(int i = 0; i < bytes && j != story.end(); ++i) {
					buf[i] = *j;
					++j;
				}

				// output the character
				const SDL_Rect rect = font::draw_text(&screen,
									screen.screen_area(),16,
									font::NORMAL_COLOUR,buf,xpos,ypos,
									NULL,false,font::NO_MARKUP);

				buf = "";
				if(rect.h > height)
					height = rect.h;
				xpos += rect.w; 
				update_rect(rect);

				if(j == story.end())
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
				return;

			events::pump();
			screen.video().flip();

			if(!skip || j == story.end())
				SDL_Delay(20);
		}
	}

	gui::draw_solid_tinted_rectangle(0,0,screen.x()-1,screen.y()-1,0,0,0,1.0,
                                     screen.video().getSurface());

	std::cerr << "intro sequence finished...\n";
}

void show_map_scene(display& screen, config& data)
{
	//stop the screen being resized while we're in this function
	const events::resize_lock stop_resizing;

	//clear the screen
	gui::draw_solid_tinted_rectangle(0,0,screen.x()-1,screen.y()-1,0,0,0,1.0,
                                     screen.video().getSurface());


	const config* const cfg_item = data.child("bigmap");
	if(cfg_item == NULL) {
		return;
	}

	const config& cfg = *cfg_item;

	const config::child_list& dots = cfg.get_children("dot");

	const std::string& image_file = cfg["image"];

	const scoped_sdl_surface image(image::get_image(image_file,image::UNSCALED));
	const scoped_sdl_surface dot_image(image::get_image(game_config::dot_image,image::UNSCALED));
	const scoped_sdl_surface cross_image(image::get_image(game_config::cross_image,image::UNSCALED));
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

	const std::string& id = data.values["id"];
	const std::string& scenario_name = string_table[id];

	const std::string& scenario = scenario_name.empty() ? data.values["name"] :
	                                                      scenario_name;
	screen.video().flip();

	CKey key;

	for(std::vector<config*>::const_iterator d = dots.begin(); d != dots.end(); ++d){
		const std::string& xloc = (**d)["x"];
		const std::string& yloc = (**d)["y"];
		const int x = atoi(xloc.c_str());
		const int y = atoi(yloc.c_str());
		if(x < 0 || x >= image->w || y < 0 || y >= image->w)
			continue;

		SDL_Surface* img = dot_image;
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
				break;
			}

			SDL_Delay(10);

			events::pump();

			int a, b;
			const int mouse_flags = SDL_GetMouseState(&a,&b);
			if(key[SDLK_RETURN] || key[SDLK_SPACE] || mouse_flags) {
				break;
			}
		}

		if(key[SDLK_ESCAPE]) {
			break;
		}

		screen.video().flip();
	}

	if(!key[SDLK_ESCAPE]) {
		SDL_Delay(500);
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

		if(new_state && !last_state)
			break;

		last_state = new_state;

		SDL_Delay(20);
		events::pump();
	}

	//clear the screen
	gui::draw_solid_tinted_rectangle(0,0,screen.x()-1,screen.y()-1,0,0,0,1.0,
                                     screen.video().getSurface());
}
