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
	close.set_location((disp.x()/2)-(close.width()/2), map_rect.y+map_rect.h+15);


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
 	text.push_back("-   Isaac Clerencia");
 	text.push_back("-   John B. Messerly");
 	text.push_back("-   Justin Zaun (jzaun)");
 	text.push_back("-   J.R. Blain (Cowboy)");
 	text.push_back("-   Kristoffer Erlandsson (erl)");
 	text.push_back("-   Philippe Plantier (Ayin)");
 	text.push_back("-   Zas");
 	text.push_back("+ ");
 
 	text.push_back("+General Purpose Administrator");
 	text.push_back("-   Crossbow/Miyo");
 	text.push_back("+ ");

 	text.push_back("+Artwork and graphics designers");
	text.push_back("-   Andrew James Patterson (Kamahawk)");
	text.push_back("-   Cedric Duval");
	text.push_back("-   Christophe Anjard");
	text.push_back("-   Diego Brea (Cobretti)");
	text.push_back("-   Gareth Miller (Gafgarion)");
	text.push_back("-   James Barton (Sangel)");
	text.push_back("-   Jimmy Olsson (Azlan)");
 	text.push_back("-   Johanna Manninen (lohari)");
	text.push_back("-   John Muccigrosso (Eponymous Archon)");
 	text.push_back("-   Jonatan Alamà (tin)");
 	text.push_back("-   Joseph Simmons (Turin)");
	text.push_back("-   J.W.C. McNabb (Darth Fool)");
 	text.push_back("-   Neorice");
 	text.push_back("-   Slainte");
 	text.push_back("-   Svetac");
 	text.push_back("+ ");

 	text.push_back("+Music");
 	text.push_back("-   Aleksi");
 	text.push_back("-   Fredrik Lindroth");
 	text.push_back("-   Joseph Toscano (zhaymusic.com)");
 	text.push_back("-   Pau Congost");
 	text.push_back("+ ");

 	text.push_back("+Scenario Designers");
	text.push_back("-   Benjamin Drieu");
 	text.push_back("-   David White (Sirp)");
 	text.push_back("-   Francisco Muñoz (fmunoz)");
 	text.push_back("-   Joseph Simmons (Turin)");
 	text.push_back("-   Justin Zaun (jzaun)");
 	text.push_back("+ ");

 	text.push_back("+Packagers");
 	text.push_back("-   Marcin Konicki (ahwayakchih)");
 	text.push_back("-   Cyril Bouthors (CyrilB)");
 	text.push_back("-   Darryl Dixon");
 	text.push_back("-   Marcus Phillips (Sithrandel)");
 	text.push_back("-   Mark Michelsen (skovbaer)");
 	text.push_back("+ ");

 	text.push_back("+Miscellaneous");
 	text.push_back("-   Jaramir");
 	text.push_back("-   Jordà Polo (ettin)");
 	text.push_back("-   Tom Chance (telex4)");
 	text.push_back("+ ");
  
 	text.push_back("+Internationalization Manager");
 	text.push_back("-   Mark Michelsen (skovbaer)");
 	text.push_back("+ ");
 
 	text.push_back("+Brazilian Translation");
 	text.push_back("-   Ambra Viviani Loos");
 	text.push_back("-   Michel Loos");
 	text.push_back("+ ");
 
 	text.push_back("+Catalan Translation");
 	text.push_back("-   Dan Rosàs Garcia (focks)");
 	text.push_back("+ ");
 
 	text.push_back("+Danish Translation");
 	text.push_back("-   Mark Michelsen (skovbaer)");
 	text.push_back("+ ");
 
 	text.push_back("+Dutch Translation");
 	text.push_back("-   Lala");
 	text.push_back("+ ");
 
 	text.push_back("+Finnish Translation");
 	text.push_back("-   paxed");
 	text.push_back("+ ");
 
 	text.push_back("+French Translation");
 	text.push_back("-   Benoit Astruc");
 	text.push_back("-   Guillaume Duwelz-Rebert");
 	text.push_back("-   DaringTremayne");
 	text.push_back("-   Zas");
 	text.push_back("+ ");
 
 	text.push_back("+German Translation");
 	text.push_back("-   Arndt Muehlenfeld");
 	text.push_back("-   ja-el");
 	text.push_back("-   Jonas");
 	text.push_back("-   ammoq");
 	text.push_back("+ ");
 
 	text.push_back("+Hungarian Translation");
 	text.push_back("-   Khiraly");
 	text.push_back("+ ");
 
 	text.push_back("+Italian Translation");
 	text.push_back("-   crys0000");
 	text.push_back("-   Federico Tomassetti");
 	text.push_back("-   isazi");
 	text.push_back("-   RokStar");
 	text.push_back("+ ");
 
 	text.push_back("+Norwegian Translation");
 	text.push_back("-   Hallvard Norheim Bø (Lysander)");
 	text.push_back("-   Erik J. Mesoy (Circon)");
 	text.push_back("+ ");
 
 	text.push_back("+Polish Translation");
 	text.push_back("-   Artur R. Czechowski");
 	text.push_back("-   methinks");
 	text.push_back("-   BOrsuk");
 	text.push_back("+ ");
 
 	text.push_back("+Portuguese Translation");
 	text.push_back("-   Celso Goya");
 	text.push_back("-   Renato Cunha");
 	text.push_back("+ ");
 
 	text.push_back("+Slovak Translation");
 	text.push_back("-   Viliam Bur");
 	text.push_back("+ ");
 
 	text.push_back("+Spanish Translation");
 	text.push_back("-   Franciso Muñoz (fmunoz)");
 	text.push_back("-   Jordà Polo (ettin)");
 	text.push_back("-   Jose Gordillo (kilder)");
 	text.push_back("-   Jose Manuel Gomez (joseg)");
 	text.push_back("+ ");
 
 	text.push_back("+Swedish Translation");
 	text.push_back("-   Alexander Kjäll (capitol)");
 	text.push_back("-   wint3r");
 	text.push_back("+ ");
 
  	text.push_back("+Past Contributors");
 	text.push_back("-   edge");
 	text.push_back("-   Frédéric Wagner");
 	text.push_back("-   Jan Zvánovec (jaz)");
 	text.push_back("-   Jay Hopping");
 	text.push_back("+ ");

	//substitute in the correct control characters for '+' and '-'
	for(std::vector<std::string>::iterator itor = text.begin(); itor != text.end(); ++itor) {
		if(itor->empty() == false) {
			if((*itor)[0] == '-') {
				(*itor)[0] = font::SMALL_TEXT;
			} else if((*itor)[0] == '+') {
				(*itor)[0] = font::LARGE_TEXT;
			}
		}
	}

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

		// handle events
		events::pump();
		events::raise_process_event();
		events::raise_draw_event();

		// update screen and wait, so the text does not scroll too fast
		update_rect(map_rect);
		disp.video().flip();
		SDL_Delay(20);

	} while(!close.pressed());

}

}
