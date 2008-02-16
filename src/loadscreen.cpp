/* $Id$ */
/*
   Copyright (C) 2005 - 2008 by Joeri Melis <joeri_melis@hotmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file loadscreen.cpp
//! Screen with logo and "Loading ..."-progressbar during program-startup.

#include "loadscreen.hpp"

#include "font.hpp"
#include "marked-up_text.hpp"
#include "gettext.hpp"
#include "filesystem.hpp"

#include <iostream>

#define MIN_PERCENTAGE   0
#define MAX_PERCENTAGE 100

loadscreen::global_loadscreen_manager::global_loadscreen_manager(CVideo& screen)
  : owns(global_loadscreen == NULL)
{
	if(owns) {
		global_loadscreen = new loadscreen(screen);
		global_loadscreen->clear_screen();
	}
}

loadscreen::global_loadscreen_manager::~global_loadscreen_manager()
{
	if(owns && global_loadscreen) {
		global_loadscreen->clear_screen();
		delete global_loadscreen;
		global_loadscreen = NULL;
	}
}

loadscreen::loadscreen(CVideo &screen, const int &percent):
	filesystem_counter(0),
	binarywml_counter(0),
	setconfig_counter(0),
	parser_counter(0),
	screen_(screen),
	logo_drawn_(false),
	pby_offset_(0),
	prcnt_(percent)
{
	std::string path = get_binary_file_location("images","misc/logo.png");
	logo_surface_ = IMG_Load(path.c_str());
		if (!logo_surface_) {
			std::cerr << "loadscreen: Failed to load the logo: " << path << std::endl;
		}
	textarea_.x = textarea_.y = textarea_.w = textarea_.h = 0;
}
void loadscreen::set_progress(const int percentage, const std::string &text, const bool commit)
{
	//! Saturate percentage.
	prcnt_ = percentage < MIN_PERCENTAGE ? MIN_PERCENTAGE: percentage > MAX_PERCENTAGE ? MAX_PERCENTAGE: percentage;
	// Set progress bar parameters:
	int fcr =  21, fcg =  53, fcb =  80;		// RGB-values for finished piece.
	int lcr =  21, lcg =  22, lcb =  24;		// Leftover piece.
	int bcr = 188, bcg = 176, bcb = 136;		// Border color.
	int bw = 1;								//!< Border width.
	int bispw = 1;								//!< Border inner spacing width.
	bw = 2*(bw+bispw) > screen_.getx() ? 0: 2*(bw+bispw) > screen_.gety() ? 0: bw;
	int scrx = screen_.getx() - 2*(bw+bispw);	//!< Available width.
	int scry = screen_.gety() - 2*(bw+bispw);	//!< Available height.
	int pbw = scrx/2;							//!< Used width.
	int pbh = scry/16;							//!< Used heigth.
	surface const gdis = screen_.getSurface();
	SDL_Rect area;
	// Draw logo if it was succesfully loaded.
	if (logo_surface_ && !logo_drawn_) {
		area.x = (screen_.getx () - logo_surface_->w) / 2;
		area.y = ((scry - logo_surface_->h) / 2) - pbh;
		area.w = logo_surface_->w;
		area.h = logo_surface_->h;
		// Check if we have enough pixels to display it.
		if (area.x > 0 && area.y > 0) {
			pby_offset_ = (pbh + area.h)/2;
			SDL_BlitSurface (logo_surface_, 0, gdis, &area);
		} else {
			std::cerr << "loadscreen: Logo image is too big." << std::endl;
		}
		logo_drawn_ = true;
		SDL_UpdateRect(gdis, area.x, area.y, area.w, area.h);
	}
	int pbx = (scrx - pbw)/2;					// Horizontal location.
	int pby = (scry - pbh)/2 + pby_offset_;		// Vertical location.

	// Draw top border.
	area.x = pbx; area.y = pby;
	area.w = pbw + 2*(bw+bispw); area.h = bw;
	SDL_FillRect(gdis,&area,SDL_MapRGB(gdis->format,bcr,bcg,bcb));
	// Draw bottom border.
	area.x = pbx; area.y = pby + pbh + bw + 2*bispw;
	area.w = pbw + 2*(bw+bispw); area.h = bw;
	SDL_FillRect(gdis,&area,SDL_MapRGB(gdis->format,bcr,bcg,bcb));
	// Draw left border.
	area.x = pbx; area.y = pby + bw;
	area.w = bw; area.h = pbh + 2*bispw;
	SDL_FillRect(gdis,&area,SDL_MapRGB(gdis->format,bcr,bcg,bcb));
	// Draw right border.
	area.x = pbx + pbw + bw + 2*bispw; area.y = pby + bw;
	area.w = bw; area.h = pbh + 2*bispw;
	SDL_FillRect(gdis,&area,SDL_MapRGB(gdis->format,bcr,bcg,bcb));
	// Draw the finished bar area.
	area.x = pbx + bw + bispw; area.y = pby + bw + bispw;
	area.w = (prcnt_ * pbw) / (MAX_PERCENTAGE - MIN_PERCENTAGE); area.h = pbh;
	SDL_FillRect(gdis,&area,SDL_MapRGB(gdis->format,fcr,fcg,fcb));
	// Draw the leftover bar area.
	area.x = pbx + bw + bispw + (prcnt_ * pbw) / (MAX_PERCENTAGE - MIN_PERCENTAGE); area.y = pby + bw + bispw;
	area.w = ((MAX_PERCENTAGE - prcnt_) * pbw) / (MAX_PERCENTAGE - MIN_PERCENTAGE); area.h = pbh;
	SDL_FillRect(gdis,&area,SDL_MapRGB(gdis->format,lcr,lcg,lcb));
	// Clear the last text and draw new if text is provided.
	if(text.length() > 0 && commit)
	{
		SDL_Rect oldarea = textarea_;
		SDL_FillRect(gdis,&textarea_,SDL_MapRGB(gdis->format,0,0,0));
		textarea_ = font::line_size(text, font::SIZE_NORMAL);
		textarea_.x = scrx/2 + bw + bispw - textarea_.w / 2;
		textarea_.y = pby + pbh + 4*(bw + bispw);
		textarea_ = font::draw_text(&screen_,textarea_,font::SIZE_NORMAL,font::NORMAL_COLOUR,text,textarea_.x,textarea_.y);
		oldarea.x = minimum<int>(textarea_.x, oldarea.x);
		oldarea.y = minimum<int>(textarea_.y, oldarea.y);
		oldarea.w = maximum<int>(textarea_.w, oldarea.w);
		oldarea.h = maximum<int>(textarea_.h, oldarea.h);
		SDL_UpdateRect(gdis, oldarea.x, oldarea.y, oldarea.w, oldarea.h);
	}
	// Update the rectangle if needed
	if(commit)
	{
		SDL_UpdateRect(gdis, pbx, pby, pbw + 2*(bw + bispw), pbh + 2*(bw + bispw));
	}
}

void loadscreen::increment_progress(const int percentage, const std::string &text, const bool commit) {
	set_progress(prcnt_ + percentage, text, commit);
}

void loadscreen::clear_screen(const bool commit)
{
	int scrx = screen_.getx();					//!< Screen width.
	int scry = screen_.gety();					//!< Screen height.
	SDL_Rect area = {0, 0, scrx, scry};		// Screen area.
	surface const disp(screen_.getSurface());	// Screen surface.
	// Make everything black.
	SDL_FillRect(disp,&area,SDL_MapRGB(disp->format,0,0,0));
	if(commit)
	{
		SDL_Flip(disp);						// Flip the double buffering.
	}
}

loadscreen *loadscreen::global_loadscreen = 0;

// Amount of work to expect during the startup-stages,
// for scaling the progressbars:
#define CALLS_TO_FILESYSTEM 112
#define PRCNT_BY_FILESYSTEM  20
#define CALLS_TO_BINARYWML 9561
#define PRCNT_BY_BINARYWML   20
#define CALLS_TO_SETCONFIG  306
#define PRCNT_BY_SETCONFIG   30
#define CALLS_TO_PARSER   50448
#define PRCNT_BY_PARSER      20

void increment_filesystem_progress () {
	unsigned newpct, oldpct;
	// Only do something if the variable is filled in.
	// I am assuming non parallel access here!
	if (loadscreen::global_loadscreen != 0) {
		if (loadscreen::global_loadscreen->filesystem_counter == 0) {
			loadscreen::global_loadscreen->increment_progress(0, _("Verifying cache."));
		}
		oldpct = (PRCNT_BY_FILESYSTEM * loadscreen::global_loadscreen->filesystem_counter) / CALLS_TO_FILESYSTEM;
		newpct = (PRCNT_BY_FILESYSTEM * ++(loadscreen::global_loadscreen->filesystem_counter)) / CALLS_TO_FILESYSTEM;
		//std::cerr << "Calls " << num;
		if(oldpct != newpct) {
			//std::cerr << " percent " << newpct;
			loadscreen::global_loadscreen->increment_progress(newpct - oldpct);
		}
		//std::cerr << std::endl;
	}
}

void increment_binary_wml_progress () {
	unsigned newpct, oldpct;
	// Only do something if the variable is filled in.
	// I am assuming non parallel access here!
	if (loadscreen::global_loadscreen != 0) {
		if (loadscreen::global_loadscreen->binarywml_counter == 0) {
			loadscreen::global_loadscreen->increment_progress(0, _("Reading cache."));
		}
		oldpct = (PRCNT_BY_BINARYWML * loadscreen::global_loadscreen->binarywml_counter) / CALLS_TO_BINARYWML;
		newpct = (PRCNT_BY_BINARYWML * ++(loadscreen::global_loadscreen->binarywml_counter)) / CALLS_TO_BINARYWML;
		//std::cerr << "Calls " << num;
		if(oldpct != newpct) {
			//std::cerr << " percent " << newpct;
			loadscreen::global_loadscreen->increment_progress(newpct - oldpct);
		}
		//std::cerr << std::endl;
	}
}

void increment_set_config_progress () {
	unsigned newpct, oldpct;
	// Only do something if the variable is filled in.
	// I am assuming non parallel access here!
	if (loadscreen::global_loadscreen != 0) {
		if (loadscreen::global_loadscreen->setconfig_counter == 0) {
			loadscreen::global_loadscreen->increment_progress(0, _("Reading unit files."));
		}
		oldpct = (PRCNT_BY_SETCONFIG * loadscreen::global_loadscreen->setconfig_counter) / CALLS_TO_SETCONFIG;
		newpct = (PRCNT_BY_SETCONFIG * ++(loadscreen::global_loadscreen->setconfig_counter)) / CALLS_TO_SETCONFIG;
		//std::cerr << "Calls " << num;
		if(oldpct != newpct) {
			//std::cerr << " percent " << newpct;
			loadscreen::global_loadscreen->increment_progress(newpct - oldpct);
		}
		//std::cerr << std::endl;
	}
}

void increment_parser_progress () {
	unsigned newpct, oldpct;
	// Only do something if the variable is filled in.
	// I am assuming non parallel access here!
	if (loadscreen::global_loadscreen != 0) {
		if (loadscreen::global_loadscreen->parser_counter == 0) {
			loadscreen::global_loadscreen->increment_progress(0, _("Reading files and creating cache."));
		}
		oldpct = (PRCNT_BY_PARSER * loadscreen::global_loadscreen->parser_counter) / CALLS_TO_PARSER;
		newpct = (PRCNT_BY_PARSER * ++(loadscreen::global_loadscreen->parser_counter)) / CALLS_TO_PARSER;
		//std::cerr << "Calls " << loadscreen::global_loadscreen->parser_counter;
		if(oldpct != newpct) {
		//	std::cerr << " percent " << newpct;
			loadscreen::global_loadscreen->increment_progress(newpct - oldpct);
		}
		//std::cerr << std::endl;
	}
}
