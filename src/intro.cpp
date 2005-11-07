/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "display.hpp"
#include "events.hpp"
#include "game_config.hpp"
#include "gettext.hpp"
#include "image.hpp"
#include "intro.hpp"
#include "font.hpp"
#include "key.hpp"
#include "log.hpp"
#include "marked-up_text.hpp"
#include "sdl_utils.hpp"
#include "sound.hpp"
#include "util.hpp"
#include "video.hpp"
#include "widgets/button.hpp"
#include "game_events.hpp"
#include "language.hpp"

#include <cstdlib>
#include <sstream>
#include <vector>

#define LOG_NG LOG_STREAM(info, engine)

namespace {
	const int min_room_at_bottom = 150;
}

bool show_intro_part(display &disp, const config& part,
		const std::string& scenario);

void show_intro(display &disp, const config& data, const config& level)
{
	LOG_NG << "showing intro sequence...\n";

	//stop the screen being resized while we're in this function
	const events::resize_lock stop_resizing;
	const events::event_context context;

	bool showing = true;

	const std::string& scenario = level["name"];

	for(config::all_children_iterator i = data.ordered_begin();
			i != data.ordered_end() && showing; i++) {
		std::pair<const std::string*, const config*> item = *i;

		if(*item.first == "part") {
			showing = show_intro_part(disp, (*item.second), scenario);
		} else if(*item.first == "if") {
			const std::string type = game_events::conditional_passed(
				NULL, item.second) ? "then":"else";
			const config* const thens = (*item.second).child(type);
			if(thens == NULL) {
				LOG_NG << "no intro story this way...\n";
				return;
			}
			const config& selection = *thens;
			show_intro(disp, selection, level);
		}
	}

	LOG_NG << "intro sequence finished...\n";
}

bool show_intro_part(display &disp, const config& part,
		const std::string& scenario)
{
	LOG_NG << "showing intro part\n";

	CVideo &video = disp.video();
	const std::string& music_file = part["music"];

	//play music if available
	if(music_file != "") {
		sound::play_music(music_file);
	}

	CKey key;
	bool		lang_rtl = current_language_rtl();

	gui::button next_button(video,_("Next") + std::string(">>>"));
	gui::button skip_button(video,_("Skip"));

	draw_solid_tinted_rectangle(0,0,video.getx()-1,video.gety()-1,
			0,0,0,1.0,video.getSurface());


	const std::string& background_name = part["background"];
	const bool show_title = (part["show_title"] == "yes");

	surface background(NULL);
	if(background_name.empty() == false) {
		background.assign(image::get_image(background_name,image::UNSCALED));
	}

	int textx = 200;
	int texty = 400;

	SDL_Rect dstrect;

	if(!background.null()) {
		dstrect.x = video.getx()/2 - background->w/2;
		dstrect.y = video.gety()/2 - background->h/2;
		dstrect.w = background->w;
		dstrect.h = background->h;

		if(dstrect.y + dstrect.h > video.gety() - min_room_at_bottom) {
			dstrect.y = maximum<int>(0,video.gety() - dstrect.h - min_room_at_bottom);
		}

		SDL_BlitSurface(background,NULL,video.getSurface(),&dstrect);

#ifdef USE_TINY_GUI
		textx = 10;
		int xbuttons = video.getx() - 50;
#else
		int xbuttons;

		if (background->w > 400) {
			textx = dstrect.x;
			xbuttons = dstrect.x+dstrect.w-40;
		} else {
			textx = 200;
			xbuttons = video.getx() - 200 - 40;
		}
#endif
		texty = dstrect.y + dstrect.h + 10;

		next_button.set_location(xbuttons,dstrect.y+dstrect.h+20);
		skip_button.set_location(xbuttons,dstrect.y+dstrect.h+70);
	} else {
		next_button.set_location(video.getx()-200,video.gety()-150);
		skip_button.set_location(video.getx()-200,video.gety()-100);
	}

	//draw title if needed
	if(show_title) {
		const SDL_Rect area = {0,0,video.getx(),video.gety()};
		const SDL_Rect scenario_size =
		      font::draw_text(NULL,area,font::SIZE_XLARGE,font::NORMAL_COLOUR,scenario,0,0);
		update_rect(font::draw_text(&video,area,font::SIZE_XLARGE,font::NORMAL_COLOUR,scenario,
					    dstrect.x,dstrect.y - scenario_size.h - 4));
	}

	update_whole_screen();
	disp.flip();

	if(!background.null()) {
		//draw images
		const config::child_list& images = part.get_children("image");

		bool pass = false;

		for(std::vector<config*>::const_iterator i = images.begin(); i != images.end(); ++i){
			const std::string& xloc = (**i)["x"];
			const std::string& yloc = (**i)["y"];
			const std::string& image_name = (**i)["file"];
			const std::string& delay_str = (**i)["delay"];
			const int delay = (delay_str == "") ? 0: atoi(delay_str.c_str());
			const int x = atoi(xloc.c_str());
			const int y = atoi(yloc.c_str());

			if(x < 0 || x >= background->w || y < 0 || y >= background->h)
				continue;

			if(image_name == "") continue;
			surface img(image::get_image(image_name,image::UNSCALED));
			if(img.null()) continue;

			if ((**i)["flip"] == "yes")
				img.assign(image::reverse_image(img));

			SDL_Rect image_rect;
			image_rect.x = x + dstrect.x;
			image_rect.y = y + dstrect.y;
			image_rect.w = img->w;
			image_rect.h = img->h;

			SDL_BlitSurface(img,NULL,video.getSurface(),&image_rect);

			update_rect(image_rect);

			if(pass == false) {
				for(int i = 0; i != 50; ++i) {
					if(key[SDLK_ESCAPE] || next_button.pressed() || skip_button.pressed()) {
						return false;
					}

					SDL_Delay(delay/50);

					events::pump();
					events::raise_process_event();
					events::raise_draw_event();

					int a, b;
					const int mouse_flags = SDL_GetMouseState(&a,&b);
					if(key[SDLK_RETURN] || key[SDLK_SPACE] || mouse_flags) {
						pass = true;
						continue;
					}

					disp.flip();
				}
			}

			if(key[SDLK_ESCAPE] || next_button.pressed() || skip_button.pressed()) {
				pass = true;
				continue;
			}
		}
	}

	const int max_width = next_button.location().x - 10 - textx;
	const std::string story = font::word_wrap_text(part["story"], font::SIZE_PLUS, max_width);
	utils::utf8_iterator itor(story);

	bool skip = false, last_key = true;

	const SDL_Rect total_size = font::draw_text(NULL, screen_area(), font::SIZE_PLUS,
			font::NORMAL_COLOUR, story, 0, 0);
	if (texty + total_size.h > screen_area().h) {
		texty = screen_area().h > total_size.h + 1 ? screen_area().h - total_size.h - 1 : 0;

		draw_solid_tinted_rectangle(textx, texty, total_size.w, total_size.h,
				0, 0, 0, 128, video.getSurface());
		update_rect(textx, texty, total_size.w, total_size.h);
	}

	if(lang_rtl)
		textx += max_width;
	int xpos = textx, ypos = texty;

	//the maximum position that text can reach before wrapping
	size_t height = 0;

	for(;;) {
		if(itor != utils::utf8_iterator::end(story)) {
			if(*itor == '\n') {
				xpos = textx;
				ypos += height;
				++itor;
			}

			// output the character
			// FIXME: this is broken: it does not take kerning into account.
			std::string tmp;
			tmp.append(itor.substr().first, itor.substr().second);
			if(lang_rtl)
				xpos -= font::line_width(tmp, font::SIZE_PLUS);
			const SDL_Rect rect = font::draw_text(&video,
					screen_area(),font::SIZE_PLUS,
					font::NORMAL_COLOUR,tmp,xpos,ypos,
					false);

			if(rect.h > height)
				height = rect.h;
			if(!lang_rtl)
				xpos += rect.w;
			update_rect(rect);

			++itor;
			if(itor == utils::utf8_iterator::end(story))
				skip = true;

		}

		const bool keydown = key[SDLK_SPACE] || key[SDLK_RETURN];

		if(keydown && !last_key || next_button.pressed()) {
			if(skip == true || itor == utils::utf8_iterator::end(story)) {
				break;
			} else {
				skip = true;
			}
		}

		last_key = keydown;

		if(key[SDLK_ESCAPE] || skip_button.pressed())
			return false;

		events::pump();
		events::raise_process_event();
		events::raise_draw_event();
		disp.flip();

		if(!skip || itor == utils::utf8_iterator::end(story))
			SDL_Delay(20);
	}

	draw_solid_tinted_rectangle(0,0,video.getx()-1,video.gety()-1,0,0,0,1.0,
                                     video.getSurface());

	return true;
}

void the_end(display &disp)
{
	SDL_Rect area = screen_area();
	CVideo &video = disp.video();
	SDL_FillRect(video.getSurface(),&area,0);

	update_whole_screen();
	disp.flip();

	const std::string text = _("The End");
	const size_t font_size = font::SIZE_XLARGE;

	area = font::text_area(text,font_size);
	area.x = screen_area().w/2 - area.w/2;
	area.y = screen_area().h/2 - area.h/2;

	for(size_t n = 0; n < 255; n += 5) {
		const SDL_Color col = {n,n,n,n};
		font::draw_text(&video,area,font_size,col,text,area.x,area.y);
		update_rect(area);
		disp.flip();

		SDL_FillRect(video.getSurface(),&area,0);

		SDL_Delay(10);
	}

	SDL_Delay(4000);
}
