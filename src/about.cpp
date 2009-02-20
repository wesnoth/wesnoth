/* $Id$ */
/*
   Copyright (C) 2003 - 2009 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file about.cpp
 * Show screen with scrolling credits.
 */

#include "about.hpp"
#include "config.hpp"
#include "construct_dialog.hpp"
#include "display.hpp"
#include "font.hpp"
#include "game_config.hpp"
#include "gettext.hpp"
#include "marked-up_text.hpp"
#include "video.hpp"
#include "show_dialog.hpp"

/**
 * @namespace about
 * Display credits %about all contributors.
 *
 * This module is used from the startup screen. \n
 * When show_about() is called, a list of contributors
 * to the game will be presented to the user.
 */
namespace about
{
	static config about_list = config();
	static std::map<std::string , std::string> images;
	static std::string images_default;

/**
 * Given a vector of strings, and a config representing an [about] section,
 * add all the credits lines from the about section to the list of strings.
 */
static void add_lines(std::vector<std::string> &res, config const &c) {
	std::string title=c["title"];
	if(title.size()) {
		title = N_("+" + title);
		res.push_back(title);
	}

	std::vector<std::string> lines = utils::split(c["text"], '\n');
	for(std::vector<std::string>::iterator line = lines.begin();
		line != lines.end(); line++) {
		if((*line)[0] == '+' && (*line).size()>1){
			*line = N_("+  " + (*line).substr(1,(*line).size()-1));
	    } else {
			*line = "-  " + *line;
	    }
	    if(line->size()) {
			if ((*line)[0] == '_')
				*line = gettext(line->substr(1,line->size()-1).c_str());
			res.push_back(*line);
		}
	}

	config::child_list entries = c.get_children("entry");
	config::child_list::const_iterator entry;
	for(entry = entries.begin(); entry != entries.end(); entry++) {
		res.push_back("-  "+(**entry)["name"]);
	}
}


std::vector<std::string> get_text(std::string campaign) {
	std::vector< std::string > res;

	const config::child_list& children = about::about_list.get_children("about");

	for(config::child_list::const_iterator cc = children.begin(); cc != children.end(); ++cc) {
	  // just finished a particular campaign
	  if(campaign.size() && campaign == (**cc)["id"]){
		add_lines(res, **cc);
	  }
	}

	for(config::child_list::const_iterator acc = children.begin(); acc != children.end(); ++acc) {
	  add_lines(res, **acc);
	}

	return res;
}

void set_about(const config& cfg){
	config::child_list about = cfg.get_children("about");
	for(config::child_list::const_iterator A = about.begin(); A != about.end(); A++) {
		config AA = (**A);
		about_list.add_child("about",AA);
		if(!AA["images"].empty()){
			if(images_default.empty()){
				images_default=AA["images"];
			}else{
				images_default+=","+AA["images"];
			}
		}
	}
	config::child_list campaigns = cfg.get_children("campaign");
	for(config::child_list::const_iterator C = campaigns.begin(); C != campaigns.end(); C++) {
		config::child_list about = (**C).get_children("about");
		if(about.size()){
			config temp;
			std::string text;
			std::string title;
			std::string campaign=(**C)["id"];
			title=(**C)["name"];
			temp["title"]=title;
			temp["id"]=(**C)["id"];
			for(config::child_list::const_iterator A = about.begin(); A != about.end(); A++) {
				config AA = (**A);
				std::string subtitle=AA["title"];
				if(subtitle.size()){
				if (subtitle[0] == '_')
						subtitle = gettext(subtitle.substr(1,subtitle.size()-1).c_str());
					text += "+" + subtitle + "\n";
				}
				std::vector<std::string> lines=utils::split(AA["text"],'\n');
				for(std::vector<std::string>::iterator line=lines.begin();
					line != lines.end(); line++){
					text+="    "+(*line)+"\n";
				}

				config::child_list entries = AA.get_children("entry");
				config::child_list::const_iterator entry;
				for(entry = entries.begin(); entry != entries.end(); entry++) {
					text+="    "+(**entry)["name"]+"\n";
				}

				if(!AA["images"].empty()){
					if(images[campaign].empty()){
						images[campaign]=AA["images"];
					}else{
						images[campaign]+=","+AA["images"];
					}
				}
			}
			temp["text"]=text;
			about_list.add_child("about",temp);
		}
	}
}

/**
 * Show credits with list of contributors.
 *
 * Names of people are shown scrolling up like in movie-credits.\n
 * Uses map from wesnoth or campaign as background.
 */
void show_about(display &disp, std::string campaign)
{
	cursor::set(cursor::WAIT);
	CVideo &video = disp.video();
	surface screen = video.getSurface();
	if (screen == NULL) return;

	std::vector<std::string> text = about::get_text(campaign);
	SDL_Rect screen_rect = {0, 0, screen->w, screen->h};

	const surface_restorer restorer(&video, screen_rect);

	cursor::set(cursor::NORMAL);

	std::vector<std::string> image_list;
	if(campaign.size() && !images[campaign].empty()){
		image_list=utils::split(images[campaign]);
	}else{
		image_list=utils::split(images_default,',',utils::STRIP_SPACES);
	}
	surface map_image(scale_surface(image::get_image(image_list[0]), screen->w, screen->h));
	if(! map_image){
		image_list[0]=game_config::game_title;
		map_image=surface(scale_surface(image::get_image(image_list[0]), screen->w, screen->h));
	}

	gui::button close(video,_("Close"));
	close.set_location((screen->w/2)-(close.width()/2), screen->h - 30);
	close.set_volatile(true);

	const int def_size = font::SIZE_XLARGE;
	const SDL_Color def_color = font::NORMAL_COLOUR;

	//substitute in the correct control characters for '+' and '-'
	std::string before_header(2, ' ');
	before_header[0] = font::LARGE_TEXT;
	for(unsigned i = 0; i < text.size(); ++i) {
		std::string &s = text[i];
		if (s.empty()) continue;
		char &first = s[0];
		if (first == '-')
			first = font::SMALL_TEXT;
		else if (first == '+') {
			first = font::LARGE_TEXT;
			text.insert(text.begin() + i, before_header);
			++i;
		}
	}
	text.insert(text.begin(), 10, before_header);

	int startline = 0;

	//TODO: use values proportionnal to screen ?
	// distance from top of map image to top of scrolling text
	const int top_margin = 60;
	// distance from bottom of scrolling text to bottom of map image
	const int bottom_margin = 40;
	// distance from left of scrolling text to the frame border
	const int text_left_padding = screen->w/32;

	int offset = 0;
	bool is_new_line = true;

	int first_line_height = 0;

	SDL_Rect frame_area = {
		screen->w * 3/32,
		top_margin,
		screen->w * 13 / 16,
		screen->h - top_margin - bottom_margin
	};

	// we use a dialog to contains the text. Strange idea but at least the style
	// will be consistent with the titlescreen
	gui::dialog_frame f(video, "", gui::dialog_frame::titlescreen_style, false);

	// set the layout and get the interior rectangle
	SDL_Rect text_rect = f.layout(frame_area).interior;
	text_rect.x += text_left_padding;
	text_rect.w -= text_left_padding;
	// make a copy to prevent SDL_blit to change its w and h
	SDL_Rect text_rect_blit = text_rect;

	CKey key;
	bool last_escape;

	surface text_surf = create_compatible_surface(screen, text_rect.w, text_rect.h);
	SDL_SetAlpha(text_surf, SDL_RLEACCEL, SDL_ALPHA_OPAQUE);

	int image_count = 0;
	int scroll_speed = 4;	// scroll_speed*50 = speed of scroll in pixel per second

	// initialy redraw all
	bool redraw_mapimage = true;
	int max_text_width = text_rect.w;

	do {
		last_escape = key[SDLK_ESCAPE] != 0;

		// check to see if background image has changed
		if(text.size() && (image_count <
				((startline * static_cast<int>(image_list.size())) /
				static_cast<int>(text.size())))){

			image_count++;
			surface temp=surface(scale_surface(image::get_image(image_list[image_count]), screen->w, screen->h));
			map_image=temp?temp:map_image;
			redraw_mapimage = true;
		}

		if (redraw_mapimage) {
			// draw map to screen, thus erasing all text
			SDL_BlitSurface(map_image, NULL, screen, NULL);
			update_rect(screen_rect);

			// redraw the dialog
			f.draw_background();
			f.draw_border();
			// cache the dialog background (alpha blending + blurred map)
			SDL_BlitSurface(screen, &text_rect, text_surf, NULL);
			redraw_mapimage = false;
		} else {
			// redraw the saved part of the dialog where text scrolled
			// thus erasing all text
			SDL_Rect modified = {0,0, max_text_width, text_rect.h};
			SDL_BlitSurface(text_surf, &modified, screen, &text_rect_blit);
			update_rect(text_rect);
		}

		const int line_spacing = 5;

		int y = text_rect.y - offset;
		int line = startline;
		int cur_line = 0;
		max_text_width = 0;

		{
		// clip to keep text into the frame (thus the new code block)
		clip_rect_setter set_clip_rect(screen, text_rect);
			do {
				// draw the text (with ellipsis if needed)
				// update the max_text_width for future cleaning
				int w = font::draw_text(&video, text_rect, def_size, def_color,
										text[line], text_rect.x, y).w;
				max_text_width = std::max<int>(max_text_width, w);
				// since the real drawing on screen is clipped,
				// we do a dummy one to get the height of the not clipped line.
				// (each time because special format characters may change it)
				const int line_height = font::draw_text(NULL, text_rect, def_size, def_color,
										text[line], 0,0).h;

				if(is_new_line) {
					is_new_line = false;
					first_line_height = line_height + line_spacing;
				}
				line++;
				if(size_t(line) > text.size()-1)
					line = 0;
				y += line_height + line_spacing;
				cur_line++;
			} while(y < text_rect.y + text_rect.h);
		}

		// performs the actual scrolling
		offset += scroll_speed;
		if (offset>=first_line_height) {
			offset -= first_line_height;
			is_new_line = true;
			startline++;
			if(size_t(startline) == text.size()){
				startline = 0;
				image_count = -1;
			}
		}

		// handle events
		if (key[SDLK_UP] && scroll_speed < 20) {
			++scroll_speed;
		}
		if (key[SDLK_DOWN] && scroll_speed > 0) {
			--scroll_speed;
		}

		events::pump();
		events::raise_process_event();
		events::raise_draw_event();

		// flip screen and wait, so the text does not scroll too fast
		disp.flip();
		disp.delay(20);

	} while(!close.pressed() && (last_escape || !key[SDLK_ESCAPE]));
}

} // end namespace about
