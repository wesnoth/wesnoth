#ifndef JM_LOADSCREEN_HPP
#define JM_LOADSCREEN_HPP

/*
   Copyright (C) 2005 by Joeri Melis <joeri_melis@hotmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "SDL.h"
#include "SDL_image.h"

#include "font.hpp"
#include "video.hpp"

#include <iostream>
#include <string>

class loadscreen {
	public:
		// Preferred constructor
		loadscreen(CVideo &screen, const int &percent = 0) :
		filesystem_counter(0),
		binarywml_counter(0),
		setconfig_counter(0),
		parser_counter(0),
		screen_(screen),
		logo_drawn_(false),
		pby_offset_(0),
		prcnt_(percent)
		{
#ifdef WESNOTH_PATH
			const char *path = WESNOTH_PATH;
#else
			const char *path = ".";
#endif
			size_t sl = strlen (path);
			char *sp = new char [sl + 21u + 1u];
			sp [0] = '\0'; 
			strncat (sp, path, sl);
			strncat (sp, "/images/misc/logo.png", 21u);
			logo_surface_ = IMG_Load (sp);
			if (!logo_surface_) {
				std::cerr << "loadscreen: Failed to load the logo: " << sp << std::endl;
			}
			delete sp;
		}
		// Keep default copy constructor
		// Keep default copy assignment
		// Destructor, dumps the counter values to stderr
		~loadscreen()
		{
			std::cerr << "loadscreen: filesystem counter = " << filesystem_counter << std::endl;
			std::cerr << "loadscreen: binarywml counter = " << binarywml_counter << std::endl;
			std::cerr << "loadscreen: setconfig counter = " << setconfig_counter << std::endl;
			std::cerr << "loadscreen: parser counter = " << parser_counter << std::endl;
			if (logo_surface_) {
				SDL_FreeSurface (logo_surface_);
			}
		}

		// Function to display a load progress bar.
		void set_progress(const int percentage=0, const std::string &text="", const bool commit=true);
		// Function to increment the progress bar.
		void increment_progress(const int percentage=1, const std::string &text="", const bool commit=true);
		// Function to draw a blank screen.
		void clear_screen(const bool commit=true);

		// Counters
		int filesystem_counter;
		int binarywml_counter;
		int setconfig_counter;
		int parser_counter;

		// A global loadscreen instance that can be used to avoid 
		// passing it on to functions that are many levels deep.		
		static loadscreen *global_loadscreen;
	private:
		// Prohibit default constructor
		loadscreen();

		// Data members
		CVideo &screen_;
		SDL_Rect textarea_;
		SDL_Surface *logo_surface_;
		bool logo_drawn_;
		int pby_offset_;
		int prcnt_;
};

// Global accessible functions that centralize the loadscreen related work.
void increment_filesystem_progress();
void increment_binary_wml_progress();
void increment_set_config_progress();
void increment_parser_progress();

#endif
