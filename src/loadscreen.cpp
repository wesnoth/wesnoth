/*
   Copyright (C) 2005 - 2016 by Joeri Melis <joeri_melis@hotmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * Screen with logo and "Loading ..."-progressbar during program-startup.
 */

#include "loadscreen.hpp"

#include "log.hpp"
#include "font.hpp"
#include "marked-up_text.hpp"
#include "gettext.hpp"
#include "sdl/rect.hpp"
#include "video.hpp"
#include "image.hpp"
#include "text.hpp"
#if SDL_VERSION_ATLEAST(2,0,0)
#include "display.hpp"
#endif

#include <SDL_events.h>
#include <SDL_image.h>

#include <cassert>

static lg::log_domain log_display("display");
static lg::log_domain log_loadscreen("loadscreen");
#define LOG_LS LOG_STREAM(info, log_loadscreen)
#define ERR_DP LOG_STREAM(err, log_display)

loadscreen::global_loadscreen_manager* loadscreen::global_loadscreen_manager::manager = NULL;

loadscreen::global_loadscreen_manager::global_loadscreen_manager(CVideo& screen)
  : owns(global_loadscreen == NULL)
{
	if(owns) {
		manager = this;
		global_loadscreen = new loadscreen(screen);
		global_loadscreen->clear_screen();
	}
}

loadscreen::global_loadscreen_manager::~global_loadscreen_manager()
{
	reset();
}

void loadscreen::global_loadscreen_manager::reset()
{
	if(owns) {
		owns = false;
		manager = NULL;
		assert(global_loadscreen);
		global_loadscreen->clear_screen();
		delete global_loadscreen;
		global_loadscreen = NULL;
	}
}

loadscreen::loadscreen(CVideo &screen, const int percent):
	screen_(screen),
	textarea_(),
#ifdef SDL_GPU
	logo_image_(image::get_texture("misc/logo-bg.png~BLIT(misc/logo.png)")),
#else
	logo_surface_(image::get_image("misc/logo-bg.png~BLIT(misc/logo.png)")),
#endif
	logo_drawn_(false),
	pby_offset_(0),
	prcnt_(percent)
{
#ifdef SDL_GPU
	if (logo_image_.null()) {
		ERR_DP << "loadscreen: Failed to load the logo" << std::endl;
	}
#else
	if (logo_surface_.null()) {
		ERR_DP << "loadscreen: Failed to load the logo" << std::endl;
	}
#endif
	textarea_.x = textarea_.y = textarea_.w = textarea_.h = 0;
}
void loadscreen::draw_screen(const std::string &text)
{
	if (screen_.faked()) return; // We seem to encounter segfault in the test executable if this is not done

	// Set progress bar parameters:
	//
	// RGB-values for finished piece.
	int fcr =  21, fcg =  53, fcb =  80;
	// Groove.
	int lcr =  21, lcg =  22, lcb =  24;
	// Border color.
	int bcr = 188, bcg = 176, bcb = 136;
	// Border width.
	int bw = 1;
	// Border inner spacing width.
	int bispw = 1;
	bw = 2*(bw+bispw) > screen_.getx() ? 0: 2*(bw+bispw) > screen_.gety() ? 0: bw;
	// Available width.
	int scrx = screen_.getx() - 2*(bw+bispw);
	// Available height.
	int scry = screen_.gety() - 2*(bw+bispw);
	// Used width.
	int pbw = scrx/2;
	// Used height.
	int pbh = scry/16;
	// Height of the lighting line.
	int	lightning_thickness = 2;

#ifdef SDL_GPU
	SDL_Rect area;

	// Pump events and make sure to redraw the logo if there's a chance that it's been obscured
	SDL_Event ev;
	while(SDL_PollEvent(&ev)) {
		if(ev.type == SDL_VIDEORESIZE || ev.type == SDL_VIDEOEXPOSE)
		{
			logo_drawn_ = false;
		}
	}

	// Draw logo if it was successfully loaded.
	if (!logo_image_.null() && !logo_drawn_) {
		area.x = (screen_.getx () - logo_image_.width()) / 2;
		area.y = ((scry - logo_image_.height()) / 2) - pbh;
		area.w = logo_image_.width();
		area.h = logo_image_.height();
		// Check if we have enough pixels to display it.
		if (area.x > 0 && area.y > 0) {
			pby_offset_ = (pbh + area.h)/2;
			screen_.draw_texture(logo_image_, area.x, area.y);
		} else {
			if (!screen_.faked()) {  // Avoid error if --nogui is used.
				ERR_DP << "loadscreen: Logo image is too big." << std::endl;
			}
		}
		logo_drawn_ = true;
	}
	int pbx = (scrx - pbw)/2;					// Horizontal location.
	int pby = (scry - pbh)/2 + pby_offset_;		// Vertical location.

	// Draw top border.
	area.x = pbx; area.y = pby;
	area.w = pbw + 2*(bw+bispw); area.h = bw;
	sdl::fill_rect(screen_, area, bcr, bcg, bcb);
	// Draw bottom border.
	area.x = pbx; area.y = pby + pbh + bw + 2*bispw;
	area.w = pbw + 2*(bw+bispw); area.h = bw;
	sdl::fill_rect(screen_, area, bcr, bcg, bcb);
	// Draw left border.
	area.x = pbx; area.y = pby + bw;
	area.w = bw; area.h = pbh + 2*bispw;
	sdl::fill_rect(screen_, area, bcr, bcg, bcb);
	// Draw right border.
	area.x = pbx + pbw + bw + 2*bispw; area.y = pby + bw;
	area.w = bw; area.h = pbh + 2*bispw;
	sdl::fill_rect(screen_, area, bcr, bcg, bcb);
	// Draw the finished bar area.
	area.x = pbx + bw + bispw; area.y = pby + bw + bispw;
	area.w = (prcnt_ * pbw) / 100; area.h = pbh;
	sdl::fill_rect(screen_, area, fcr, fcg, fcb);

	SDL_Rect lightning = area;
	lightning.h = lightning_thickness;
	//we add 25% of white to the color of the bar to simulate a light effect
	sdl::fill_rect(screen_, lightning, (fcr*3+255)/4, (fcg*3+255)/4, (fcb*3+255)/4);
	lightning.y = area.y+area.h-lightning.h;
	//remove 50% of color to simulate a shadow effect
	sdl::fill_rect(screen_, lightning, fcr/2, fcg/2, fcb/2);

	// Draw the leftover bar area.
	area.x = pbx + bw + bispw + (prcnt_ * pbw) / 100; area.y = pby + bw + bispw;
	area.w = ((100 - prcnt_) * pbw) / 100; area.h = pbh;
	sdl::fill_rect(screen_, area, lcr, lcg, lcb);

	// Clear the last text and draw new if text is provided.
	if (!text.empty())
	{
		sdl::fill_rect(screen_, textarea_, 0, 0, 0);
		font::ttext label;
		label.set_text(text, false);
		sdl::timage txt = label.render_as_texture();
		textarea_.w = txt.width();
		textarea_.h = txt.height();
		textarea_.x = scrx/2 + bw + bispw - textarea_.w / 2;
		textarea_.y = pby + pbh + 4*(bw + bispw);
		screen_.draw_texture(txt, textarea_.x, textarea_.y);
	}
	screen_.flip();
#else
	surface& gdis = screen_.getSurface();
	SDL_Rect area;

	// Pump events and make sure to redraw the logo if there's a chance that it's been obscured
	SDL_Event ev;
	while(SDL_PollEvent(&ev)) {
#if SDL_VERSION_ATLEAST(2,0,0)
		if (ev.type == SDL_WINDOWEVENT &&
				ev.window.event == SDL_WINDOWEVENT_RESIZED) {
			screen_.update_framebuffer();
		}
		if (ev.type == SDL_WINDOWEVENT &&
				(ev.window.event == SDL_WINDOWEVENT_RESIZED ||
						ev.window.event == SDL_WINDOWEVENT_EXPOSED))
#else
		if(ev.type == SDL_VIDEORESIZE || ev.type == SDL_VIDEOEXPOSE)
#endif
		{
			logo_drawn_ = false;
		}
	}

	// Draw logo if it was successfully loaded.
	if (logo_surface_ && !logo_drawn_) {
		area.x = (screen_.getx () - logo_surface_->w) / 2;
		area.y = ((scry - logo_surface_->h) / 2) - pbh;
		area.w = logo_surface_->w;
		area.h = logo_surface_->h;
		// Check if we have enough pixels to display it.
		if (area.x > 0 && area.y > 0) {
			pby_offset_ = (pbh + area.h)/2;
			sdl_blit(logo_surface_, 0, gdis, &area);
		} else {
			if (!screen_.faked()) {  // Avoid error if --nogui is used.
				ERR_DP << "loadscreen: Logo image is too big." << std::endl;
			}
		}
		logo_drawn_ = true;
		update_rect(area.x, area.y, area.w, area.h);
	}
	int pbx = (scrx - pbw)/2;					// Horizontal location.
	int pby = (scry - pbh)/2 + pby_offset_;		// Vertical location.

	// Draw top border.
	area.x = pbx; area.y = pby;
	area.w = pbw + 2*(bw+bispw); area.h = bw;
	sdl::fill_rect(gdis,&area,SDL_MapRGB(gdis->format,bcr,bcg,bcb));
	// Draw bottom border.
	area.x = pbx; area.y = pby + pbh + bw + 2*bispw;
	area.w = pbw + 2*(bw+bispw); area.h = bw;
	sdl::fill_rect(gdis,&area,SDL_MapRGB(gdis->format,bcr,bcg,bcb));
	// Draw left border.
	area.x = pbx; area.y = pby + bw;
	area.w = bw; area.h = pbh + 2*bispw;
	sdl::fill_rect(gdis,&area,SDL_MapRGB(gdis->format,bcr,bcg,bcb));
	// Draw right border.
	area.x = pbx + pbw + bw + 2*bispw; area.y = pby + bw;
	area.w = bw; area.h = pbh + 2*bispw;
	sdl::fill_rect(gdis,&area,SDL_MapRGB(gdis->format,bcr,bcg,bcb));
	// Draw the finished bar area.
	area.x = pbx + bw + bispw; area.y = pby + bw + bispw;
	area.w = (prcnt_ * pbw) / 100; area.h = pbh;
	sdl::fill_rect(gdis,&area,SDL_MapRGB(gdis->format,fcr,fcg,fcb));

	SDL_Rect lightning = area;
	lightning.h = lightning_thickness;
	//we add 25% of white to the color of the bar to simulate a light effect
	sdl::fill_rect(gdis,&lightning,SDL_MapRGB(gdis->format,(fcr*3+255)/4,(fcg*3+255)/4,(fcb*3+255)/4));
	lightning.y = area.y+area.h-lightning.h;
	//remove 50% of color to simulate a shadow effect
	sdl::fill_rect(gdis,&lightning,SDL_MapRGB(gdis->format,fcr/2,fcg/2,fcb/2));

	// Draw the leftover bar area.
	area.x = pbx + bw + bispw + (prcnt_ * pbw) / 100; area.y = pby + bw + bispw;
	area.w = ((100 - prcnt_) * pbw) / 100; area.h = pbh;
	sdl::fill_rect(gdis,&area,SDL_MapRGB(gdis->format,lcr,lcg,lcb));

	// Clear the last text and draw new if text is provided.
	if (!text.empty())
	{
		SDL_Rect oldarea = textarea_;
		sdl::fill_rect(gdis,&textarea_,SDL_MapRGB(gdis->format,0,0,0));
		textarea_ = font::line_size(text, font::SIZE_NORMAL);
		textarea_.x = scrx/2 + bw + bispw - textarea_.w / 2;
		textarea_.y = pby + pbh + 4*(bw + bispw);
		textarea_ = font::draw_text(&screen_,textarea_,font::SIZE_NORMAL,font::NORMAL_COLOR,text,textarea_.x,textarea_.y);
		SDL_Rect refresh = sdl::union_rects(oldarea, textarea_);
		update_rect(refresh.x, refresh.y, refresh.w, refresh.h);
	}
	// Update the rectangle.
	update_rect(pbx, pby, pbw + 2*(bw + bispw), pbh + 2*(bw + bispw));
	screen_.flip();
#endif
}

void loadscreen::clear_screen()
{
#ifdef SDL_GPU
	GPU_Clear(get_render_target());
#else
	int scrx = screen_.getx();                     // Screen width.
	int scry = screen_.gety();                     // Screen height.
	SDL_Rect area = sdl::create_rect(0, 0, scrx, scry); // Screen area.
	surface& disp(screen_.getSurface());      // Screen surface.
	// Make everything black.
	sdl::fill_rect(disp,&area,SDL_MapRGB(disp->format,0,0,0));
	update_whole_screen();
	screen_.flip();
#endif
}

loadscreen *loadscreen::global_loadscreen = 0;

struct load_stage
{
	char const *id;
	char const *name;
	int start_pos, max_count;
};

static int const nb_stages = 19;

/**
 * Description of all the stages.
 * @note Some of the stages appear twice; this is not a mistake. It
 *       accounts for their cost at title time and at game time.
 * @note The values have been automatically generated by running a cache-hot
 *       Wesnoth on HttH with the --log-info=loadscreen option.
 */
static load_stage const stages[nb_stages] =
{
	{ "init gui", N_("Initializing user interface"), 0, 27089 },
	{ "load config", N_("Loading game configuration"), 46, 0 },
	{ "verify cache", N_("Verifying cache"), 46, 179 },
	{ "create cache", N_("Reading files and creating cache"), 47, 60317 },
	{ "load unit types", N_("Reading unit files"), 85, 531 },
	{ "init fonts", N_("Reinitialize fonts for the current language"), 96, 21 },
	{ "refresh addons", N_("Searching for installed add-ons"), 99, 0 },
	{ "titlescreen", N_("Loading title screen"), 100, 0 },
	{ "load data", N_("Loading data files"), 0, 0 },
	{ "verify cache", N_("Verifying cache"), 0, 0 },
	{ "create cache", N_("Reading files and creating cache"), 0, 152852 },
	{ "load unit types", N_("Reading unit files"), 38, 553 },
	{ "load level", N_("Loading level"), 41, 0 },
	{ "init teams", N_("Initializing teams"), 41, 0 },
	{ "load units", N_("Loading units"), 43, 0 },
	{ "init theme", N_("Initializing display"), 43, 0 },
	{ "build terrain", N_("Building terrain rules"), 43, 1545 },
	{ "init display", N_("Initializing display"), 99, 0 },
	{ "start game", N_("Starting game"), 100, 0 },
};

static int current_stage;
static int stage_counter[nb_stages];
static unsigned stage_time[nb_stages];

void loadscreen::start_stage(char const *id)
{
	assert(global_loadscreen);

	int s = -1;
	for (int i = 0; i < nb_stages; ++i) {
		int j = (i + current_stage) % nb_stages;
		if (strcmp(id, stages[j].id) == 0) {
			s = j;
			break;
		}
	}
	assert(s >= 0);

	const load_stage &cs = stages[s];
	global_loadscreen->prcnt_ = cs.start_pos;
	global_loadscreen->draw_screen(translation::gettext(cs.name));
	stage_counter[s] = 0;
	stage_time[s] = SDL_GetTicks();
	current_stage = s;
}

void loadscreen::increment_progress()
{
	if (!global_loadscreen) return;

	int v = ++stage_counter[current_stage];
	int m = stages[current_stage].max_count;
	if (v > m) return;

	int s = stages[current_stage].start_pos;
	int percentage = s + v * (stages[current_stage + 1].start_pos - s) / m;
	if (percentage == global_loadscreen->prcnt_) return;

	global_loadscreen->prcnt_ = percentage;
	global_loadscreen->draw_screen(std::string());
}

void loadscreen::dump_counters() const
{
	if (lg::info.dont_log(log_loadscreen)) return;

	std::ostringstream s;

	int i = 0;
	while (i < nb_stages)
	{
		int j;
		for (j = i; stages[j].start_pos < 100; ++j) {}
		if (stage_time[i] == stage_time[j]) break;
		for (int k = i; k <= j; ++k)
		{
			int v = stages[k].start_pos;
			if (i < k && k < j) {
				v = stages[i].start_pos +
					(100 - stages[i].start_pos) *
					(stage_time[k] - stage_time[i]) /
					(stage_time[j] - stage_time[i]);
			}
			s << "\t{ \"" << stages[k].id << "\", N_(\""
			  << stages[k].name << "\"), " << v << ", "
			  << stage_counter[k] << " },\n";
		}
		i = j + 1;
	}
	LOG_LS << "Suggested loadscreen values:\n---\n" << s.str() << "---\n";
}
