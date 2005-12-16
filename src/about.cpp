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
#include "events.hpp"
#include "font.hpp"
#include "game_config.hpp"
#include "gettext.hpp"
#include "image.hpp"
#include "key.hpp"
#include "display.hpp"
#include "sdl_utils.hpp"
#include "video.hpp"
#include "widgets/button.hpp"

#include <sstream>

namespace about
{

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
		"-   Bram Ridder (Morloth)",
		"-   Bruno Wolff III",
		"-   Cedric Duval",
		"-   Cyril Bouthors (CyrilB)",
		"-   Guillaume Melquiond (silene)",
		"-   Isaac Clerencia",
		"-   Jan Zvánovec (jaz)",
		"-   John B. Messerly",
		"-   Jon Daniel (forcemstr)",
		"-   Justin Zaun (jzaun)",
		"-   J.R. Blain (Cowboy)",
		"-   Kristoffer Erlandsson (erl)",
		"-   Maksim Orlovich (SadEagle)",
		"-   Philippe Plantier (Ayin)",
		"-   Yann Dirson",
		"-   Zas",

		"_" N_("+General Purpose Administrators"),
		"-   Crossbow/Miyo",
		"-   Isaac Clerencia",

		"_" N_("+Artwork and graphics designers"),
		"-   Alex Jarocha-Ernst (Jormungandr)",
		"-   Andrew James Patterson (Kamahawk)",
		"-   antwerp",
		"-   Christophe Anjard",
		"-   Diego Brea (Cobretti)",
		"-   Eli Dupree (Elvish Pillager)",
		"-   Gareth Miller (Gafgarion)",
		"-   Hogne Håskjold (frame)",
		"-   James Barton (Sangel)",
		"-   James Woo (Pickslide)",
		"-   Jason Lutes",
		"-   Jimmy Olsson (Azlan)",
		"-   Johanna Manninen (lohari)",
		"-   John Muccigrosso (Eponymous Archon)",
		"-   John-Robert Funck (XJaPaN)",
		"-   Jonatan Alamà (tin)",
		"-   Joseph Simmons (Turin)",
		"-   J.W. Bjerk (Eleazar)",
		"-   J.W.C. McNabb (Darth Fool)",
		"-   Michael Gil de Muro (grp21)",
		"-   Neorice",
		"-   Peter Geinitz (Shadow)",
		"-   Richard Kettering (Jetryl)",
		"-   Slainte",
		"-   Stephen Stone (Disto)",
		"-   Svetac",

		"_" N_("+Music"),
		"-   Aleksi",
		"-   Fredrik Lindroth",
		"-   Joseph Toscano (zhaymusic.com)",
		"-   Pau Congost",

		"_" N_("+Scenario Designers"),
		"-   Benjamin Drieu",
		"-   Dacyn",
		"-   David White (Sirp)",
		"-   Francisco Muñoz (fmunoz)",
		"-   James Spencer (Shade)",
		"-   Joseph Simmons (Turin)",
		"-   Justin Zaun (jzaun)",

		"_" N_("+Multiplayer Maps"),
		"-   Mike Quinones (Doc Paterson)",
		"-   Peter Groen (pg)",
		"-   Tom Chance (telex4)",

		"_" N_("+Packagers"),
		"-   Cyril Bouthors (CyrilB)",
		"-   Darryl Dixon",
		"-   edge",
		"-   Isaac Clerencia",
		"-   Jay Hopping",
		"-   Marcin Konicki (ahwayakchih)",
		"-   Marcus Phillips (Sithrandel)",
		"-   Mark Michelsen (skovbaer)",

		"_" N_("+Miscellaneous"),
		"-   Bartek Waresiak (Dragonking)",
		"-   Francesco Gigli (Jaramir)",
		"-   Jordà Polo (ettin)",
		"-   Richard S. (Noy)",
		"-   Ruben Philipp Wickenhäuser (The Very Uhu)",

		"_" N_("+Internationalization Managers"),
		"-   Cédric Duval",
		"-   David Philippi (Torangan)",
		"-   Mark Michelsen (skovbaer)",
		"-   Nils Kneuper (Ivanovic)",
		"-   Susanna Björverud (sanna)",

		"_" N_("+Afrikaans Translation"),
		"-   András Salamon (ott)",
		"-   Erhard Eiselen",
		"-   Nico Oliver (nicoza)",
		"-   Renier Maritz",

		"_" N_("+Basque Translation"),
		"-   Alfredo Beaumont (ziberpunk)",
		"-   Julen Landa (genars)",
		"-   Mikel Olasagasti (Hey_neken)",

		"_" N_("+Bulgarian Translation"),
		"-   Anton Tsigularov (Atilla)",
		"-   Georgi Dimitrov (oblak)",

		"_" N_("+Catalan Translation"),
		"-   Carles Company (brrr)",
		"-   Dan Rosàs Garcia (focks)",
		"-   Daniel López (Azazelo)",
		"-   Jonatan Alamà (tin)",
		"-   Jordà Polo (ettin)",
		"-   Jose Gordillo (kilder)",
		"-   Mark Recasens",
		"-   Pau Rul·lan Ferragut",

		"_" N_("+Chinese Translation"),
		"-   林俊杰 - Lim Choon Kiat",

		"_" N_("+Czech Translation"),
		"-   Anežka Bubení?ková (Bubu)",
		"-   David Nečas (Yeti)",
		"-   Lukáš Faltýnek",
		"-   Martin Šín",
		"-   Mintaka",
                "-   Oto Buchta (tapik)",
		"-   Petr Kopač (Ferda)",
		"-   Petr Kovár (Juans)",
		"-   Rudolf Orság",
		"-   Sofronius",
		"-   Vít Komárek",
		"-   Vít Krčál",
		"-   Vladimír Slávik",

		"_" N_("+Danish Translation"),
		"-   Anders K. Madsen (madsen)",
		"-   Bjarke Sørensen (basher)",
		"-   Jesper Fuglsang Wolff (ulven)",
		"-   Mark Michelsen (skovbaer)",
		"-   Mathias Bundgaard Svensson (freaken)",

		"_" N_("+Dutch Translation"),
		"-   Arne Deprez",
		"-   Lala",
		"-   Maarten Albrecht",
		"-   Pieter Vermeylen (Onne)",
                "-   Roel Thijs (Roel)",
		"-   Tobe Deprez",

		"_" N_("+English (GB) Translation"),
		"-   András Salamon (ott)",

		"_" N_("+Estonian Translation"),
		"-   Mart Tõnso",

		"_" N_("+Finnish Translation"),
		"-   Ankka",
		"-   kko",
		"-   Matias Parmala",
		"-   paxed",

		"_" N_("+French Translation"),
		"-   Aurélien Brevers (Breversa)",
		"-   Benoit Astruc",
		"-   Cédric Duval",
		"-   DaringTremayne",
		"-   François Orieux",
		"-   Guillaume Duwelz-Rebert",
		"-   Guillaume Massart (Piou2fois)",
		"-   Guillaume Melquiond (silene)",
		"-   Jean Privat (Tout)",
		"-   Jean-Luc Richard (Le Gnome)",
		"-   Jérémy Rosen (Boucman)",
		"-   Julien Moncel",
		"-   Julien Tailleur",
		"-   Nicolas Boudin (Blurgk)",
		"-   Philippe Plantier (Ayin)",
		"-   Yann Dirson",
		"-   Zas",

		"_" N_("+German Translation"),
		"-   Andre Schmidt (schmidta)",
		"-   Boris Stumm (quijote_)",
		"-   Christoph Berg (chrber)",
		"-   Gerfried Fuchs (Alfie)",
		"-   Jan Greve (Jan)",
		"-   Jan Heiner Laberenz (jan-heiner)",
		"-   Kai Ensenbach (Pingu)",
		"-   Nils Kneuper (Ivanovic)",
		"-   Ruben Philipp Wickenhäuser (The Very Uhu)",
		"-   Stephan Grochtmann (Schattenstephan)",

		"_" N_("+Greek Translation"),
		"-   Katerina Sykioti",
		"-   Konstantinos Karasavvas",
                "-   Spiros, Giorgis",
                "-   Alexander Alexiou (Santi)",

		"_" N_("+Hungarian Translation"),
		"-   adson",
		"-   Beer (Eddi)",
		"-   dentro",
		"-   Gilluin",
		"-   Kékkői László (BlackEvil)",
		"-   Kertész Csaba",
		"-   Khiraly",
		"-   Kovács Dániel",
		"-   krix",
		"-   Salamon András (ott)",
		"-   Széll Tamás (TomJoad)",

		"_" N_("+Italian Translation"),
		"-   Alessio D'Ascanio (otaku)",
		"-   Americo Iacovizzi (DarkAmex)",
		"-   crys0000",
		"-   Eugenio Favalli (ElvenProgrammer)",
		"-   Federico Tomassetti",
		"-   isazi",
		"-   RokStar",

		"_" N_("+Japanese Translation"),
		"-   いいむらなおき (amatubu) - Naoki Iimura",
		"-   岡田信人 - Nobuhito Okada",
		"-   Yuji Matsumoto",

		"_" N_("+Latin Translation"),
		"-   Mark Polo (mpolo)",

		"_" N_("+Norwegian Translation"),
		"-   Hallvard Norheim Bø (Lysander)",
		"-   Håvard Korsvoll",
		"-   Erik J. Mesoy (Circon)",
		"-   Susanne Mesoy (Rarlgland)",

		"_" N_("+Polish Translation"),
		"-   Arkadiusz Danilecki (szopen)",
		"-   Artur R. Czechowski",
		"-   Bartek Waresiak (Dragonking)",
		"-   BOrsuk",
		"-   Karol Nowak (grzywacz)",
		"-   methinks",
		"-   Michał Jedynak (Artanis)",
		"-   Paweł Stradomski",
		"-   Paweł Tomak",

		"_" N_("+Portuguese (Brazil) Translation"),
		"-   Ambra Viviani Loos",
		"-   Celso Goya",
		"-   Claus Aranha",
		"-   Michel Loos",
		"-   Renato Cunha",
		"-   Sérgio de Miranda Costa",
		"-   Tiago Souza (Salvador)",

		"_" N_("+Russian Translation"),
		"-   Alexandr Menovchicov",
		"-   Azamat Hackimov",
		"-   Ilya Kaznacheev",
		"-   Roman Tuchin (Sankt)",

		"_" N_("+Serbian Translation"),
		"-   Srecko Toroman (FreeCraft)",

		"_" N_("+Slovak Translation"),
		"-   Viliam Bur",

		"_" N_("+Slovenian Translation"),
		"-   Jaka Kranjc (lynx)",

		"_" N_("+Spanish Translation"),
		"-   David Martínez Moreno",
		"-   Flamma",
		"-   Francisco Muñoz (fmunoz)",
		"-   Gabriel Rodríguez (Chewie)",
		"-   Iván Herrero (navitux)",
		"-   Jose Gordillo (kilder)",
		"-   Jose Manuel Gomez (joseg)",

		"_" N_("+Swedish Translation"),
		"-   Alexander Kjäll (capitol)",
		"-   Leo Danielson (Lugo Moll)",
		"-   Stefan Bergström (tephlon)",
		"-   Susanna Björverud (sanna)",
		"-   wint3r",

		"_" N_("+Turkish Translation"),
		"-   Enes Akın (yekialem)",
		"-   İhsan Akın",
		"-   Kosif",
		"-   Selim Farsakoğlu",

		"_" N_("+Contributors"),
		"-   Frédéric Wagner",

		"_" N_("+Bots"),
		"-   wesbot",
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
	return res;
}

void show_about(display &disp)
{
	CVideo &video = disp.video();
	std::vector<std::string> text = get_text();
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
