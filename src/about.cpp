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

namespace about
{

std::vector<std::string> get_text() {
	static const std::string credits[] = {
		" ",
		"- ",
		" ",
		"- ",
		" ",
		"- ",
		" ",
		"- ",
		" ",
		"- ",
		" ",
		"- ",

		_("+Core Developers"),
		_("-   Main Developer"),
		"   David White (Sirp)",
		"- ",
		_("-   Artwork and graphics designer"),
		"   Francisco Muñoz (fmunoz)",
		"+ ",

		_("+Developers"),
		"-   Alfredo Beaumont (ziberpunk)",
		"-   Cyril Bouthors (CyrilB)",
		"-   Guillaume Melquiond (silene)",
		"-   Isaac Clerencia",
		"-   John B. Messerly",
		"-   Justin Zaun (jzaun)",
		"-   J.R. Blain (Cowboy)",
		"-   Kristoffer Erlandsson (erl)",
		"-   Philippe Plantier (Ayin)",
		"-   Yann Dirson",
		"-   Zas",
		"+ ",

		_("+General Purpose Administrators"),
		"-   Cedric Duval",
		"-   Crossbow/Miyo",
		"+ ",

		_("+Artwork and graphics designers"),
		"-   Andrew James Patterson (Kamahawk)",
		"-   antwerp",
		"-   Christophe Anjard",
		"-   Diego Brea (Cobretti)",
		"-   Eli Dupree (Elvish Pillager)",
		"-   Gareth Miller (Gafgarion)",
		"-   Hogne Håskjold (frame)",
		"-   James Barton (Sangel)",
		"-   Jimmy Olsson (Azlan)",
		"-   Johanna Manninen (lohari)",
		"-   John Muccigrosso (Eponymous Archon)",
		"-   John-Robert Funck (XJaPaN)",
		"-   Jonatan Alamà (tin)",
		"-   Joseph Simmons (Turin)",
		"-   J.W. Bjerk (Eleazar)",
		"-   J.W.C. McNabb (Darth Fool)",
		"-   Neorice",
		"-   Richard Kettering (Jetryl)",
		"-   Slainte",
		"-   Svetac",
		"+ ",

		_("+Music"),
		"-   Aleksi",
		"-   Fredrik Lindroth",
		"-   Joseph Toscano (zhaymusic.com)",
		"-   Pau Congost",
		"+ ",

		_("+Scenario Designers"),
		"-   Benjamin Drieu",
		"-   David White (Sirp)",
		"-   Francisco Muñoz (fmunoz)",
		"-   James Spencer (Shade)",
		"-   Joseph Simmons (Turin)",
		"-   Justin Zaun (jzaun)",
		"+ ",

		_("+Packagers"),
		"-   Darryl Dixon",
		"-   Jay Hopping",
		"-   Marcin Konicki (ahwayakchih)",
		"-   Marcus Phillips (Sithrandel)",
		"-   Mark Michelsen (skovbaer)",
		"+ ",

		_("+Miscellaneous"),
		"-   Francesco Gigli (Jaramir)",
		"-   Jordà Polo (ettin)",
		"-   Tom Chance (telex4)",
		"+ ",

		_("+Internationalization Manager"),
		"-   Mark Michelsen (skovbaer)",
		"+ ",

		_("+Basque Translation"),
		"-   Alfredo Beaumont (ziberpunk)",
		"+ ",

		_("+Catalan Translation"),
		"-   Dan Rosàs Garcia (focks)",
		"-   Jordà Polo (ettin)",
		"+ ",

		_("+Czech Translation"),
		"-   David Nečas (Yeti)",
		"-   Mintaka",
		"-   Petr Kopač (Ferda)",
		"-   Petr Kovár (Juans)",
		"-   Sofronius",
		"-   Vít Krčál",
		"+ ",

		_("+Danish Translation"),
		"-   Mark Michelsen (skovbaer)",
		"-   Mathias Bundgaard Svensson (freaken)",
		"+ ",

		_("+Dutch Translation"),
		"-   Lala",
		"-   Pieter Vermeylen (Onne)",
		"+ ",

		_("+Finnish Translation"),
		"-   paxed",
		"+ ",

		_("+French Translation"),
		"-   Benoit Astruc",
		"-   Cédric Duval",
		"-   Guillaume Duwelz-Rebert",
		"-   Guillaume Melquiond (silene)",
		"-   Jean-Luc Richard (Le Gnome)",
		"-   DaringTremayne",
		"-   Philippe Plantier (Ayin)",
		"-   Tout",
		"-   Zas",
		"+ ",

		_("+German Translation"),
		"-   Andre Schmidt",
		"-   Boris Stumm",
		"-   Christoph Berg",
		"-   Jan Heiner",
		"-   Kai Ensenbach",
		"-   Nils Kneuper",
		"-   Stephan Grochtmann",
		"+ ",

		_("+Hungarian Translation"),
		"-   Khiraly",
		"-   Széll Tamás (TomJoad)",
		"+ ",

		_("+Italian Translation"),
		"-   crys0000",
		"-   Federico Tomassetti",
		"-   isazi",
		"-   RokStar",
		"+ ",

		_("+Norwegian Translation"),
		"-   Hallvard Norheim Bø (Lysander)",
		"-   Erik J. Mesoy (Circon)",
		"-   Susanne Mesoy (Rarlgland)",
		"+ ",

		_("+Polish Translation"),
		"-   Artur R. Czechowski",
		"-   methinks",
		"-   BOrsuk",
		"+ ",

		_("+Portuguese (Brazil) Translation"),
		"-   Ambra Viviani Loos",
		"-   Celso Goya",
		"-   Claus Aranha",
		"-   Michel Loos",
		"-   Renato Cunha",
		"+ ",

		_("+Slovak Translation"),
		"-   Viliam Bur",
		"+ ",

		_("+Spanish Translation"),
		"-   David Martínez",
		"-   Franciso Muñoz (fmunoz)",
		"-   Gabriel Rodríguez (Chewie)",
		"-   Jose Gordillo (kilder)",
		"-   Jose Manuel Gomez (joseg)",
		"+ ",

		_("+Swedish Translation"),
		"-   Alexander Kjäll (capitol)",
		"-   Susanna Björverud (sanna)",
		"-   Stefan Bergström (tephlon)",
		"-   wint3r",
		"+ ",
		
		_("+Contributors"),
		"-   Cyril Bouthors (CyrilB)",
		"-   edge",
		"-   Frédéric Wagner",
		"-   Jan Zvánovec (jaz)",
		"-   Maksim Orlovich (SadEagle)",
		"+ ",

		_("+ Bots"),
		"-   wesbot",
		"+ "
	};

	return std::vector<std::string>(credits, credits + sizeof(credits) / sizeof(*credits));
}

void show_about(display& disp)
{
	std::vector<std::string> text = get_text();
	SDL_Rect rect = {0, 0, disp.x(), disp.y()};

	const surface_restorer restorer(&disp.video(), rect);

	// Clear the screen
	gui::draw_solid_tinted_rectangle(0,0,disp.x()-1,disp.y()-1,
	                                 0,0,0,1.0,disp.video().getSurface());
	update_whole_screen();

	const surface map_image(image::get_image(game_config::map_image,image::UNSCALED));
	SDL_Rect map_rect;
	map_rect.x = disp.x()/2 - map_image->w/2;
	map_rect.y = disp.y()/2 - map_image->h/2;
	map_rect.w = map_image->w;
	map_rect.h = map_image->h;

	gui::button close(disp,_("Close"));
	close.set_location((disp.x()/2)-(close.width()/2), map_rect.y+map_rect.h+15);


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
