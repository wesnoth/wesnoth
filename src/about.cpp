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
	text.push_back("- ");
	text.push_back("David White (Sirp)");
	text.push_back("-main developer");
	text.push_back("-scenario designer");
	text.push_back("Francisco Muñoz (fmunoz)");
	text.push_back("-artwork and graphics designer");
	text.push_back("- ");
	text.push_back("- ");
	text.push_back("- ");
	text.push_back("+Active Contributors");
	text.push_back("- ");
	text.push_back("Alfredo Beaumont (ziberpunk)");
	text.push_back("-developer");
	text.push_back("Arndt Muehlenfeld (Arndt)");
	text.push_back("-translator");
	text.push_back("Crossbow (miyo)");
	text.push_back("-wesnoth general purpose administrator");
	text.push_back("Cyril Bouthors (CyrilB)");
	text.push_back("-packager");
	text.push_back("-developer");
	text.push_back("Federico Tomassetti");
	text.push_back("-translator");
	text.push_back("Guillaume Duwelz-Rebert");
	text.push_back("-translator");
	text.push_back("-developer");
	text.push_back("Jaramir");
	text.push_back("-web developer");
	text.push_back("Johanna Manninen (lohari)");
	text.push_back("-artwork and graphics designer");
	text.push_back("Jordà Polo (ettin)");
	text.push_back("-web developer");
	text.push_back("-artwork and graphics designer");
	text.push_back("Joseph Toscano (zhaymusic.com)");
	text.push_back("-music");
	text.push_back("Justin Zaun (jzaun)");
	text.push_back("-developer");
	text.push_back("-scenario designer");
	text.push_back("J.R. Blain (Cowboy)");
	text.push_back("-developer");
	text.push_back("Marcus Phillips (Sithrandel)");
	text.push_back("-packager");
	text.push_back("Mark Joakim Bekker Michelsen (skovbaer)");
	text.push_back("-internationalization manager");
	text.push_back("-translator");
	text.push_back("Pau Congost");
	text.push_back("-music");
	text.push_back("Fredrik Lindroth");
	text.push_back("-music");
	text.push_back("Slainte");
	text.push_back("-artwork and graphics designer");
	text.push_back("Zas");
	text.push_back("-translator");
	text.push_back("-developer");
	text.push_back("- ");
	text.push_back("- ");
	text.push_back("- ");
	text.push_back("+Past Contributors");
	text.push_back("- ");
	text.push_back("Jan Zvánovec (jaz)");
	text.push_back("-developer");
	text.push_back("Jay Hopping");
	text.push_back("-artwork and graphics designer");

	int startline = 0;
	int timer = 0;
	for(;;) {
		events::pump();

		int mousex, mousey;
		const int mouse_flags = SDL_GetMouseState(&mousex,&mousey);
		const bool left_button = mouse_flags&SDL_BUTTON_LMASK;

		SDL_BlitSurface(map_image,NULL,disp.video().getSurface(),&map_rect);
		update_rect(map_rect);

		int height = map_rect.y + 60;
		int line = startline;
		for(int cur_line=0;cur_line<12;cur_line++)
		{
			if(size_t(line) > text.size()-1)
				line=0;
			SDL_Rect tr = font::draw_text(&disp,disp.screen_area(),24,font::BLACK_COLOUR,
					              text[line], -1,height);
			line++;
			height += tr.h + 5;
		}
		timer++;
		if(timer == 20)
		{
			timer = 0;
			startline++;
		}
		if(size_t(startline) == text.size())
			startline = 0;

		if(close.process(mousex,mousey,left_button)) {
			return;
		}

		disp.video().flip();
		SDL_Delay(20);
	}

}

}
