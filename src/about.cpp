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

#include "about.hpp"
#include "display.hpp"
#include "font.hpp"
#include "game_config.hpp"
#include "gettext.hpp"
#include "marked-up_text.hpp"
#include "video.hpp"

namespace about
{

  config about_list = config();

std::vector<std::string> get_text() {
	static const char *credits[] = {
		"_" N_("+Core Developers"),
		"_" N_("-   Main Developer"),
		"   David White (Sirp)",
		"- ",
		"_" N_("-   Artwork and graphics designer"),
		"   Francisco Muñoz (fmunoz)",

		"_" N_("+Developers"),
		"-   Alfredo Beaumont (ziberpunk)",
		"-   András Salamon (ott)",
		"-   Benoît Timbert (Noyga)",
		"-   Bram Ridder (Morloth)",
		"-   Bruno Wolff III",
		"-   Cedric Duval",
		"-   Cyril Bouthors (CyrilB)",
		"-   Dominic Bolin (Xan)",
		"-   Guillaume Melquiond (silene)",
		"-   Isaac Clerencia",
		"-   Jan Zvánovec (jaz)",
		"-   Jérémy Rosen (Boucman)",
		"-   John B. Messerly",
		"-   John W. C. McNabb (Darth Fool)",
		"-   Jon Daniel (forcemstr)",
		"-   Jörg Hinrichs (Yogi Bear/YogiHH)",
		"-   Justin Zaun (jzaun)",
		"-   J.R. Blain (Cowboy)",
		"-   Kristoffer Erlandsson (erl)",
		"-   Maksim Orlovich (SadEagle)",
		"-   Nicolas Weeger (Ryo)",
		"-   Philippe Plantier (Ayin)",
		"-   Rusty Russell (rusty)",
		"-   Yann Dirson",
		"-   Zas",
		"_" N_("+General Purpose Administrators"),
		"-   Crossbow/Miyo",
		"-   Isaac Clerencia",
	};

	std::vector< std::string > res;
	size_t len = sizeof(credits) / sizeof(*credits);
	res.reserve(len);
	for(size_t i = 0; i < len; ++i) {
		const char *s = credits[i];
		if (s[0] == '_')
		  s = gettext(s + 1);
		res.push_back(s);
	}

	const config::child_list& children = about::about_list.get_children("about");
	for(config::child_list::const_iterator cc = children.begin(); cc != children.end(); ++cc) {
	  std::string title=(**cc)["title"];
	  if(title.size()){
	    title = N_("+" + title);
	    res.push_back(title);
	  }
	  std::vector<std::string> lines=utils::split((**cc)["text"],'\n');
	  for(std::vector<std::string>::iterator line=lines.begin();
	      line != lines.end(); line++){
	    if((*line)[0] == '+' && (*line).size()>1){
	      *line = N_("+  " + (*line).substr(1,(*line).size()-1));
	    }else{
	      *line = "-  " + *line;
	    }
	    if(line->size()){
	      if ((*line)[0] == '_')
		*line = gettext(line->substr(1,title.size()-1).c_str());
	      res.push_back(*line);
	    }
	  } 
	}

	return res;
}

void set_about(const config& cfg){
  config::child_list campaigns = cfg.get_children("campaign");
  for(config::child_list::const_iterator C = campaigns.begin(); C != campaigns.end(); C++) {
    config::child_list about = (**C).get_children("about");
    if(about.size()){
      config temp;
      std::string text;
      std::string title;
      title=(**C)["name"];
      temp["title"]=title;
      for(config::child_list::const_iterator A = about.begin(); A != about.end(); A++) {
	config AA = (**A);
	//	text+="+   " + AA["title"] +"\n";
	text+=AA["title"]+"\n";
	std::vector<std::string> lines=utils::split(AA["text"],'\n');
	for(std::vector<std::string>::iterator line=lines.begin();
		      line != lines.end(); line++){
	  text+="    "+(*line)+"\n";
	}
      }
      temp["text"]=text;
      about_list.add_child("about",temp);
    }
  }

  config::child_list about = cfg.get_children("about");
  for(config::child_list::const_iterator A = about.begin(); A != about.end(); A++) {
    about_list.add_child("about",(**A));
  }
}

void show_about(display &disp)
{
	CVideo &video = disp.video();
	std::vector<std::string> text = about::get_text();
	SDL_Rect rect = {0, 0, video.getx(), video.gety()};

	const surface_restorer restorer(&video, rect);

	// Clear the screen
	draw_solid_tinted_rectangle(0,0,video.getx()-1,video.gety()-1,
	                                 0,0,0,1.0,video.getSurface());
	update_whole_screen();

	const surface map_image(image::get_image(game_config::map_image,image::UNSCALED));
	SDL_Rect map_rect;
	map_rect.x = video.getx()/2 - map_image->w/2;
	map_rect.y = video.gety()/2 - map_image->h/2;
	map_rect.w = map_image->w;
	map_rect.h = map_image->h;

	gui::button close(video,_("Close"));
	close.set_location((video.getx()/2)-(close.width()/2), map_rect.y+map_rect.h+15);


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

	// the following two lines should be changed if the image of the map is changed
	const int top_margin = 60;		// distance from top of map image to top of scrolling text
	const int bottom_margin = 40;	// distance from bottom of scrolling text to bottom of map image

	int offset = 0;
	bool is_new_line = true;

	int first_line_height = 0;

	// the following rectangles define the top, middle and bottom of the background image
	// the upper and lower part is later used to mask the upper and lower line of scrolling text
	SDL_Rect upper_src = {0, 0, map_rect.w, top_margin};
	SDL_Rect upper_dest = {map_rect.x, map_rect.y, map_rect.w, top_margin};
	SDL_Rect middle_src = {0, top_margin, map_rect.w, map_rect.h - top_margin - bottom_margin};
	SDL_Rect middle_dest = {map_rect.x, map_rect.y + top_margin, map_rect.w, map_rect.h - top_margin - bottom_margin};
	SDL_Rect lower_src = {0, map_rect.h - bottom_margin, map_rect.w, bottom_margin};
	SDL_Rect lower_dest = {map_rect.x, map_rect.y + map_rect.h - bottom_margin, map_rect.w, bottom_margin};

	do {
		// draw map to screen, thus erasing all text
		SDL_BlitSurface(map_image,&middle_src,video.getSurface(),&middle_dest);

		// draw one screen full of text
		const int line_spacing = 5;
		int y = map_rect.y + top_margin - offset;
		int line = startline;
		int cur_line = 0;

		do {
			SDL_Rect tr = font::draw_text(&video,screen_area(),font::SIZE_XLARGE,font::BLACK_COLOUR,
					              text[line], map_rect.x + map_rect.w / 8,y);
			if(is_new_line) {
				is_new_line = false;
				first_line_height = tr.h + line_spacing;
			}
			line++;
			if(size_t(line) > text.size()-1)
				line = 0;
			y += tr.h + line_spacing;
			cur_line++;
		} while(y<map_rect.y + map_rect.h - bottom_margin);

		// performs the actual scrolling
		const int scroll_speed = 2;		// scroll_speed*50 = speed of scroll in pixel per second

		offset += scroll_speed;
		if(offset>=first_line_height) {
			offset -= first_line_height;
			is_new_line = true;
			startline++;
			if(size_t(startline) == text.size())
				startline = 0;
		}

		// mask off the upper and lower half of the map,
		// so text will scroll into view rather than
		// suddenly appearing out of nowhere
		SDL_BlitSurface(map_image,&upper_src,video.getSurface(),&upper_dest);
		SDL_BlitSurface(map_image,&lower_src,video.getSurface(),&lower_dest);

		// handle events
		events::pump();
		events::raise_process_event();
		events::raise_draw_event();

		// update screen and wait, so the text does not scroll too fast
		update_rect(map_rect);
		disp.flip();
		SDL_Delay(20);

	} while(!close.pressed());

}

}
