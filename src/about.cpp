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
#include "key.hpp"
#include "language.hpp"
#include "display.hpp"
#include "show_dialog.hpp"
#include "sdl_utils.hpp"
#include "widgets/button.hpp"

#include <sstream>
#include <string>

namespace about
{

void show_about(display& disp)
{
	SDL_Rect rect = {0, 0, disp.x(), disp.y()};

	const surface_restorer restorer(&disp.video(), rect);

	// Clear the screen
	gui::draw_solid_tinted_rectangle(0,0,disp.x()-1,disp.y()-1,
	                                 0,0,0,1.0,disp.video().getSurface());
	update_whole_screen();

	const scoped_sdl_surface map_image(image::get_image(game_config::map_image,image::UNSCALED));
	SDL_Rect map_rect;
	map_rect.x = disp.x()/2 - map_image->w/2;
	map_rect.y = disp.y()/2 - map_image->h/2;
	map_rect.w = map_image->w;
	map_rect.h = map_image->h;

	gui::button close(disp,string_table["close_button"]);
	close.set_xy((disp.x()/2)-(close.width()/2), map_rect.y+map_rect.h+15);


	std::vector<std::string> text;
	text.push_back(" ");
	text.push_back("- ");
	text.push_back(" ");
	text.push_back("- ");
	text.push_back(" ");
	text.push_back("- ");
	text.push_back(" ");
	text.push_back("- ");
	text.push_back(" ");
	text.push_back("- ");
	text.push_back(" ");
	text.push_back("- ");

  	text.push_back("+Core Developers");
 	text.push_back("-   Main Developer");
 	text.push_back("   David White (Sirp)");
  	text.push_back("- ");
 	text.push_back("-   Artwork and graphics designer");
 	text.push_back("   Francisco Muñoz (fmunoz)");
 	text.push_back("+ ");
 
 	text.push_back("+Developers");
 	text.push_back("-   Alfredo Beaumont (ziberpunk)");
 	text.push_back("-   Cyril Bouthors (CyrilB)");
 	text.push_back("-   Guillaume Duwelz-Rebert");
 	text.push_back("-   Isaac Clerencia");
 	text.push_back("-   J.R. Blain (Cowboy)");
 	text.push_back("-   Justin Zaun (jzaun)");
 	text.push_back("-   Zas");
 	text.push_back("+ ");
 
 	text.push_back("+General Purpose Administrator");
 	text.push_back("-   Crossbow/Miyo");
 	text.push_back("+ ");
 
 	text.push_back("+Internationalization Manager");
 	text.push_back("-   Mark Joakim Bekker Michelsen (skovbaer)");
 	text.push_back("+ ");
 
 	text.push_back("+Translators");
 	text.push_back("-   Arndt Muehlenfeld (Arndt)");
 	text.push_back("-   Federico Tomassetti");
 	text.push_back("-   Guillaume Duwelz-Rebert");
 	text.push_back("-   Mark Joakim Bekker Michelsen (skovbaer)");
 	text.push_back("-   Zas");
 	text.push_back("+ ");
 
 	text.push_back("+Artwork and graphics designers");
 	text.push_back("-   Johanna Manninen (lohari)");
 	text.push_back("-   Jordà Polo (ettin)");
 	text.push_back("-   Slainte");
 	text.push_back("+ ");
 
 	text.push_back("+Music");
 	text.push_back("-   Fredrik Lindroth");
 	text.push_back("-   Joseph Toscano (zhaymusic.com)");
 	text.push_back("-   Pau Congost");
 	text.push_back("+ ");
 
 	text.push_back("+Scenario Designers");
 	text.push_back("-   David White (Sirp)");
 	text.push_back("-   Justin Zaun (jzaun)");
 	text.push_back("+ ");
 
 	text.push_back("+Packagers");
 	text.push_back("-   Cyril Bouthors (CyrilB)");
 	text.push_back("-   Marcus Phillips (Sithrandel)");
 	text.push_back("+ ");
 
 	text.push_back("+Web Developers");
 	text.push_back("-   Jaramir");
 	text.push_back("-   Jordà Polo (ettin)");
 	text.push_back("+ ");
 
  	text.push_back("+Past Contributors");
 	text.push_back("-   Jan Zvánovec (jaz)");
 	text.push_back("-   Developer");
  	text.push_back("- ");
 	text.push_back("-   Jay Hopping");
 	text.push_back("-   Artwork and graphics designer");

	int startline = 0;

	// the following two lines should be changed if the image of the map is changed
	const int top_margin = 60;		// distance from top of map image to top of scrolling text
	const int bottom_margin = 40;	// distance from bottom of scrolling text to bottom of map image

	int offset = 0;
	bool is_new_line = true;

	int mousex, mousey;
	bool left_button;
	int first_line_height;

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
		SDL_BlitSurface(map_image,&middle_src,disp.video().getSurface(),&middle_dest);

		// draw one screen full of text
		const int line_spacing = 5;
		int y = map_rect.y + top_margin - offset;
		int line = startline;
		int cur_line = 0;

		do {
			SDL_Rect tr = font::draw_text(&disp,disp.screen_area(),24,font::BLACK_COLOUR,
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
		SDL_BlitSurface(map_image,&upper_src,disp.video().getSurface(),&upper_dest);
		SDL_BlitSurface(map_image,&lower_src,disp.video().getSurface(),&lower_dest);

		// update screen and wait, so the text does not scroll too fast
		update_rect(map_rect);
		disp.video().flip();
		SDL_Delay(20);

		// handle events
		events::pump();
		left_button = SDL_GetMouseState(&mousex,&mousey)&SDL_BUTTON_LMASK;
	} while(!close.process(mousex,mousey,left_button));

}

}
