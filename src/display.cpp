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

#include "actions.hpp"
#include "display.hpp"
#include "events.hpp"
#include "font.hpp"
#include "game.hpp"
#include "game_config.hpp"
#include "hotkeys.hpp"
#include "image.hpp"
#include "language.hpp"
#include "log.hpp"
#include "preferences.hpp"
#include "sdl_utils.hpp"
#include "show_dialog.hpp"
#include "sound.hpp"
#include "team.hpp"
#include "tooltips.hpp"
#include "util.hpp"

#include "SDL_image.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <sstream>

std::map<gamemap::location,double> display::debugHighlights_;

namespace {
	const double DefaultZoom = 70.0;

	const size_t SideBarGameStatus_x = 16;
	const size_t SideBarGameStatus_y = 220;

	const SDL_Rect empty_rect = {0,0,0,0};
}

display::display(unit_map& units, CVideo& video, const gamemap& map,
				 const gamestatus& status, const std::vector<team>& t, const config& theme_cfg)
		             : screen_(video), xpos_(0.0), ypos_(0.0),
					   zoom_(DefaultZoom), map_(map), units_(units),
					   minimap_(NULL), redrawMinimap_(false),
					   pathsList_(NULL), status_(status),
                       teams_(t), lastDraw_(0), drawSkips_(0),
					   invalidateAll_(true), invalidateUnit_(true),
					   invalidateGameStatus_(true), panelsDrawn_(false),
					   currentTeam_(0), activeTeam_(0), hideEnergy_(false),
					   deadAmount_(0.0), advancingAmount_(0.0), updatesLocked_(0),
                       turbo_(false), grid_(false), sidebarScaling_(1.0),
					   theme_(theme_cfg,screen_area()), firstTurn_(true), map_labels_(*this,map)
{
	if(non_interactive())
		updatesLocked_++;

	energy_bar_rect_.x = -1;

	create_buttons();

	std::fill(reportRects_,reportRects_+reports::NUM_REPORTS,empty_rect);

	new_turn();

	image::set_zoom(zoom_);

	gameStatusRect_.w = 0;
	unitDescriptionRect_.w = 0;
	unitProfileRect_.w = 0;

	//clear the screen contents
	SDL_Surface* const disp = screen_.getSurface();
	SDL_Rect area = screen_area();
	SDL_FillRect(disp,&area,SDL_MapRGB(disp->format,0,0,0));
}

//we have to go through all this trickery on clear_surfaces because
//some compilers don't support 'typename type::iterator'
template<typename Map,typename FwIt>
void clear_surfaces_internal(Map& surfaces, FwIt beg, FwIt end)
{
	for(; beg != end; ++beg) {
		SDL_FreeSurface(beg->second);
	}

	surfaces.clear();
}

template<typename Map>
void clear_surfaces(Map& surfaces)
{
	clear_surfaces_internal(surfaces,surfaces.begin(),surfaces.end());
}

display::~display()
{
	SDL_FreeSurface(minimap_);
	prune_chat_messages(true);
}

Uint32 display::rgb(Uint8 red, Uint8 green, Uint8 blue)
{
	return 0xFF000000 | (red << 16) | (green << 8) | blue;
}

void display::new_turn()
{
	int r,g,b;
	image::get_colour_adjustment(&r,&g,&b);

	const time_of_day& tod = status_.get_time_of_day();
	const int red = tod.red - r;
	const int green = tod.green - g;
	const int blue = tod.blue - b;

	const int niterations = turbo() || firstTurn_ || (!red && !green && !blue) ? 0 : 10;
	firstTurn_ = false;

	const int frame_time = 30;
	const int starting_ticks = SDL_GetTicks();
	for(int i = 0; i != niterations; ++i) {
		image::set_colour_adjustment(r+(i*red)/niterations,g+(i*green)/niterations,b+(i*blue)/niterations);
		invalidate_all();
		draw();

		const int cur_ticks = SDL_GetTicks();
		const int wanted_ticks = starting_ticks + i*frame_time;
		if(cur_ticks < wanted_ticks) {
			SDL_Delay(wanted_ticks - cur_ticks);
		}
	}

	adjust_colours(0,0,0);
}

void display::adjust_colours(int r, int g, int b)
{
	const time_of_day& tod = status_.get_time_of_day();
	image::set_colour_adjustment(tod.red+r,tod.green+g,tod.blue+b);
}

gamemap::location display::hide_unit(const gamemap::location& loc, bool hide_energy)
{
	const gamemap::location res = hiddenUnit_;
	hiddenUnit_ = loc;
	hideEnergy_ = hide_energy;
	return res;
}

int display::x() const { return screen_.getx(); }
int display::mapx() const { return x() - 140; }
int display::y() const { return screen_.gety(); }

const SDL_Rect& display::map_area() const
{
	return theme_.main_map_location(screen_area());
}

const SDL_Rect& display::minimap_area() const
{
	return theme_.mini_map_location(screen_area());
}

SDL_Rect display::screen_area() const
{
	const SDL_Rect res = {0,0,x(),y()};
	return res;
}

void display::select_hex(gamemap::location hex)
{
	if(team_valid() && teams_[currentTeam_].fogged(hex.x,hex.y)) {
		return;
	}

	invalidate(selectedHex_);
	selectedHex_ = hex;
	invalidate(selectedHex_);
	invalidate_unit();
}

void display::highlight_hex(gamemap::location hex)
{
	const int has_unit = units_.count(mouseoverHex_) + units_.count(hex);

	invalidate(mouseoverHex_);
	mouseoverHex_ = hex;
	invalidate(mouseoverHex_);
	invalidate_game_status();

	if(has_unit)
		invalidate_unit();
}

gamemap::location display::hex_clicked_on(int xclick, int yclick)
{
	const SDL_Rect& rect = map_area();
	if(xclick < rect.x || xclick >= rect.x + rect.w ||
	   yclick < rect.y || yclick >= rect.y + rect.h)
		return gamemap::location();

	xclick -= rect.x;
	yclick -= rect.y;

	const int tile_width = static_cast<int>(static_cast<int>(zoom_)*0.75) + 1;

	const double xtile = xpos_/tile_width +
			               static_cast<double>(xclick)/tile_width - 0.25;
	const double ytile = ypos_/zoom_ + static_cast<double>(yclick)/zoom_
	                      + (is_odd(int(xtile)) ? -0.5:0.0);

	return gamemap::location(static_cast<int>(xtile),static_cast<int>(ytile));
}

double display::get_location_x(const gamemap::location& loc) const
{
	const int tile_width = static_cast<int>(static_cast<int>(zoom_)*0.75) + 1;

	return map_area().x + loc.x*tile_width - xpos_;
}

double display::get_location_y(const gamemap::location& loc) const
{
	return map_area().y + static_cast<double>(loc.y)*zoom_ - ypos_ +
					         (is_odd(loc.x) ? zoom_/2.0 : 0.0);
}

gamemap::location display::minimap_location_on(int x, int y)
{
	const SDL_Rect rect = minimap_area();

	if(x < rect.x || y < rect.y ||
	   x >= rect.x + rect.w || y >= rect.y + rect.h) {
		return gamemap::location();
	}

	const double xdiv = double(rect.w) / double(map_.x());
	const double ydiv = double(rect.h) / double(map_.y());

	return gamemap::location(int((x - rect.x)/xdiv),int((y-rect.y)/ydiv));
}

void display::scroll(double xmove, double ymove)
{
	const double orig_x = xpos_;
	const double orig_y = ypos_;
	xpos_ += xmove;
	ypos_ += ymove;
	bounds_check_position();

	if(int(util::round(xpos_)) == int(util::round(orig_x)))
		xpos_ = orig_x;

	if(int(util::round(ypos_)) == int(util::round(orig_y)))
		ypos_ = orig_y;

	//only invalidate if we've actually moved
	if(orig_x != xpos_ || orig_y != ypos_) {
		map_labels_.scroll(int(util::round(orig_x - xpos_)), int(util::round(orig_y - ypos_)));
		invalidate_all();
	}
}

void display::zoom(double amount)
{
	const double orig_xpos = xpos_;
	const double orig_ypos = ypos_;

	xpos_ /= zoom_;
	ypos_ /= zoom_;

	const double max_zoom = 200.0;
	const double orig_zoom = zoom_;

	zoom_ += amount;
	if(zoom_ > max_zoom) {
		zoom_ = orig_zoom;
		xpos_ = orig_xpos;
		ypos_ = orig_ypos;
		return;
	}

	xpos_ *= zoom_;
	ypos_ *= zoom_;

	xpos_ += amount*2;
	ypos_ += amount*2;

	const double prev_zoom = zoom_;

	bounds_check_position();

	if(zoom_ != prev_zoom) {
		map_labels_.recalculate_labels();
		xpos_ = orig_xpos;
		ypos_ = orig_ypos;
		zoom_ = orig_zoom;
		return;
	}

	energy_bar_rect_.x = -1;

	image::set_zoom(zoom_);
	invalidate_all();
}

void display::default_zoom()
{
	zoom(DefaultZoom - zoom_);
}

void display::scroll_to_tile(int x, int y, SCROLL_TYPE scroll_type, bool check_fogged)
{
	if(update_locked() || (check_fogged && fogged(x,y)))
		return;

	if(map_.on_board(gamemap::location(x,y)) == false)
		return;

	const double xpos = static_cast<double>(x)*zoom_*0.75 - xpos_;
	const double ypos = static_cast<double>(y)*zoom_ - ypos_ +
					                    ((x % 2) == 1 ? zoom_/2.0 : 0.0);

	const double speed = preferences::scroll_speed()*2.0;

	const SDL_Rect& area = map_area();
	const double desiredxpos = area.w/2.0 - zoom_/2.0;
	const double desiredypos = area.h/2.0 - zoom_/2.0;

	const double xmove = xpos - desiredxpos;
	const double ymove = ypos - desiredypos;

	int num_moves = static_cast<int>((fabs(xmove) > fabs(ymove) ?
	                                  fabs(xmove):fabs(ymove))/speed);

	if(scroll_type == WARP || turbo())
		num_moves = 1;

	const double divisor = static_cast<double>(num_moves);

	double xdiff = 0.0, ydiff = 0.0;

	for(int i = 0; i != num_moves; ++i) {
		events::pump();
		xdiff += xmove/divisor;
		ydiff += ymove/divisor;

		//accelerate scroll rate if either shift key is held down
		if((i%4) != 0 && i != num_moves-1 && turbo()) {
			continue;
		}

		scroll(xdiff,ydiff);
		draw();

		xdiff = 0.0;
		ydiff = 0.0;
	}

	invalidate_all();
	draw();
}

void display::scroll_to_tiles(int x1, int y1, int x2, int y2,
                              SCROLL_TYPE scroll_type, bool check_fogged)
{
	const double xpos1 = static_cast<double>(x1)*zoom_*0.75 - xpos_;
	const double ypos1 = static_cast<double>(y1)*zoom_ - ypos_ +
					                    ((x1 % 2) == 1 ? zoom_/2.0 : 0.0);
	const double xpos2 = static_cast<double>(x2)*zoom_*0.75 - xpos_;
	const double ypos2 = static_cast<double>(y2)*zoom_ - ypos_ +
					                    ((x2 % 2) == 1 ? zoom_/2.0 : 0.0);

	const double diffx = fabs(xpos1 - xpos2);
	const double diffy = fabs(ypos1 - ypos2);

	if(diffx > map_area().w/(zoom_*0.75) || diffy > map_area().h/zoom_) {
		scroll_to_tile(x1,y1,scroll_type,check_fogged);
	} else {
		scroll_to_tile((x1+x2)/2,(y1+y2)/2,scroll_type,check_fogged);
	}
}

void display::bounds_check_position()
{
	const double min_zoom1 = static_cast<double>(map_area().w/(map_.x()*0.75 + 0.25));
	const double min_zoom2 = static_cast<double>(map_area().h/map_.y());
	const double min_zoom = ceil(maximum<double>(min_zoom1,min_zoom2)/5.0)*5.0;
	const double max_zoom = 200.0;

	const int orig_zoom = int(zoom_);

	zoom_ = floor(zoom_);

	if(zoom_ < min_zoom) {
		zoom_ = min_zoom;
	}

	if(zoom_ > max_zoom) {
		zoom_ = max_zoom;
	}

	const int tile_width = static_cast<int>(static_cast<int>(zoom_)*0.75) + 1;

	const double xend = tile_width*map_.x() + tile_width/3;
	const double yend = zoom_*map_.y() + zoom_/2.0;

	if(xpos_ + static_cast<double>(map_area().w) > xend)
		xpos_ -= xpos_ + static_cast<double>(map_area().w) - xend;

	if(ypos_ + static_cast<double>(map_area().h) > yend)
		ypos_ -= ypos_ + static_cast<double>(map_area().h) - yend;

	if(xpos_ < 0.0)
		xpos_ = 0.0;

	if(ypos_ < 0.0)
		ypos_ = 0.0;

	if(int(zoom_) != orig_zoom)
		image::set_zoom(zoom_);
}

void display::redraw_everything()
{
	if(update_locked() || teams_.empty())
		return;

	bounds_check_position();

	for(size_t n = 0; n != reports::NUM_REPORTS; ++n) {
		reportRects_[n] = empty_rect;
		reportSurfaces_[n].assign(NULL);
		reports_[n] = reports::report();
	}

	tooltips::clear_tooltips();

	theme_.set_resolution(screen_area());
	create_buttons();

	panelsDrawn_ = false;

	map_labels_.recalculate_labels();

	invalidate_all();
	draw(true,true);
}

namespace {

void draw_panel(display& disp, const theme::panel& panel)
{
	log_scope("draw panel");
	scoped_sdl_surface surf(image::get_image(panel.image(),image::UNSCALED));

	const SDL_Rect screen = disp.screen_area();
	SDL_Rect& loc = panel.location(screen);
	if(surf->w != loc.w || surf->h != loc.h) {
		surf.assign(scale_surface(surf.get(),loc.w,loc.h));
	}

	std::cerr << "drawing panel " << loc.x << "," << loc.y << "," << loc.w << "," << loc.h << "\n";

	disp.blit_surface(loc.x,loc.y,surf);
	update_rect(loc);
}

void draw_label(display& disp, SDL_Surface* target, const theme::label& label)
{
	log_scope("draw label");

	const std::string& text = label.text();
	const std::string& icon = label.icon();
	SDL_Rect& loc = label.location(disp.screen_area());
	
	if(icon.empty() == false) {
		scoped_sdl_surface surf(image::get_image(icon,image::UNSCALED));
		if(surf->w != loc.w || surf->h != loc.h) {
			surf.assign(scale_surface(surf.get(),loc.w,loc.h));
		}

		SDL_BlitSurface(surf.get(),NULL,target,&loc);

		if(text.empty() == false) {
			tooltips::add_tooltip(loc,text);
		}
	} else if(text.empty() == false) {
		font::draw_text(&disp,loc,label.font_size(),font::NORMAL_COLOUR,text,loc.x,loc.y);
	}


	update_rect(loc);
}

}

void display::draw(bool update,bool force)
{
	if(!panelsDrawn_ && !teams_.empty()) {
		SDL_Surface* const screen = screen_.getSurface();

		const std::vector<theme::panel>& panels = theme_.panels();
		for(std::vector<theme::panel>::const_iterator p = panels.begin(); p != panels.end(); ++p) {
			draw_panel(*this,*p);
		}

		const std::vector<theme::label>& labels = theme_.labels();
		for(std::vector<theme::label>::const_iterator i = labels.begin(); i != labels.end(); ++i) {
			draw_label(*this,screen,*i);
		}
		
		for(std::vector<gui::button>::iterator b = buttons_.begin(); b != buttons_.end(); ++b) {
			b->draw();
		}

		//invalidate the reports so they are redrawn
		std::fill(reports_,reports_+sizeof(reports_)/sizeof(*reports_),reports::report());
		invalidateGameStatus_ = true;
		panelsDrawn_ = true;
	}

	if(invalidateAll_ && !map_.empty()) {
		for(int x = -1; x <= map_.x(); ++x)
			for(int y = -1; y <= map_.y(); ++y)
				draw_tile(x,y);
		invalidateAll_ = false;

		redrawMinimap_ = true;
	} else if(!map_.empty()) {
		for(std::set<gamemap::location>::const_iterator it =
		    invalidated_.begin(); it != invalidated_.end(); ++it) {
			draw_tile(it->x,it->y);
		}

		invalidated_.clear();
	}

	if(redrawMinimap_) {
		redrawMinimap_ = false;
		const SDL_Rect area = minimap_area();
		draw_minimap(area.x,area.y,area.w,area.h);
	}


	if(!map_.empty()) {
		draw_sidebar();
	}

	prune_chat_messages();

	const int max_skips = 5;
	const int time_between_draws = 20;
	const int current_time = SDL_GetTicks();
	const int wait_time = lastDraw_ + time_between_draws - current_time;

	//force a wait for 10 ms every frame.
	//TODO: review whether this is the correct thing to do
	SDL_Delay(maximum<int>(10,wait_time));

	if(update) {
		lastDraw_ = SDL_GetTicks();

		if(wait_time >= 0 || drawSkips_ >= max_skips || force)
			update_display();
		else
			drawSkips_++;

	}
}

void display::update_display()
{
	if(updatesLocked_ > 0)
		return;

	screen_.flip();
}

void display::draw_sidebar()
{
	if(teams_.empty())
		return;

	if(invalidateUnit_) {
		//we display the unit the mouse is over if it is over a unit
		//otherwise we display the unit that is selected
		std::map<gamemap::location,unit>::const_iterator i = 
			find_visible_unit(units_,mouseoverHex_,
					map_,
					status_.get_time_of_day().lawful_bonus,
					teams_,teams_[viewing_team()]);

		if(i == units_.end() || fogged(i->first.x,i->first.y)) {
			i = find_visible_unit(units_,selectedHex_,
					map_,
					status_.get_time_of_day().lawful_bonus,
					teams_,teams_[viewing_team()]);
		}

		if(i != units_.end() && !fogged(i->first.x,i->first.y))
			for(size_t r = reports::UNIT_REPORTS_BEGIN; r != reports::UNIT_REPORTS_END; ++r)
				draw_report(reports::TYPE(r));

		invalidateUnit_ = false;
	}

	if(invalidateGameStatus_) {
		draw_game_status(mapx()+SideBarGameStatus_x,SideBarGameStatus_y);
		invalidateGameStatus_ = false;
	}
}

void display::draw_game_status(int x, int y)
{
	if(teams_.empty())
		return;

	for(size_t r = reports::STATUS_REPORTS_BEGIN; r != reports::STATUS_REPORTS_END; ++r) {
		draw_report(reports::TYPE(r));
	}	
}

void display::draw_image_for_report(scoped_sdl_surface& img, scoped_sdl_surface& surf, SDL_Rect& rect)
{
	SDL_Rect visible_area = get_non_transperant_portion(img);
	if(visible_area.x != 0 || visible_area.y != 0 || visible_area.w != img->w || visible_area.h != img->h) {
		if(visible_area.w == 0 || visible_area.h == 0) {
			return;
		}

		//since we're blitting a transperant image, we need to back up
		//the surface for later restoration
		surf.assign(get_surface_portion(screen_.getSurface(),rect));

		SDL_Rect target = rect;
		if(visible_area.w > rect.w || visible_area.h > rect.h) {
			img.assign(get_surface_portion(img,visible_area));
			img.assign(scale_surface(img,rect.w,rect.h));
			visible_area.x = 0;
			visible_area.y = 0;
			visible_area.w = img->w;
			visible_area.h = img->h;
		} else {
			target.x = rect.x + (rect.w - visible_area.w)/2;
			target.y = rect.y + (rect.h - visible_area.h)/2;
			target.w = visible_area.w;
			target.h = visible_area.h;
		}

		SDL_BlitSurface(img,&visible_area,screen_.getSurface(),&target);
	} else {
		if(img->w != rect.w || img->h != rect.h) {
			img.assign(scale_surface(img,rect.w,rect.h));
		}

		SDL_BlitSurface(img,NULL,screen_.getSurface(),&rect);
	}
}

void display::draw_report(reports::TYPE report_num)
{
	if(!team_valid())
		return;

	const theme::status_item* const item = theme_.get_status_item(reports::report_name(report_num));
	if(item != NULL) {

		reports::report report = reports::generate_report(report_num,map_,
				units_, teams_,
		      teams_[viewing_team()],
				currentTeam_+1,activeTeam_+1,
				selectedHex_,mouseoverHex_,status_,observers_);

		SDL_Rect& rect = reportRects_[report_num];
		const SDL_Rect& new_rect = item->location(screen_area());

		//report and its location is unchanged since last time. Do nothing.
		if(rect == new_rect && reports_[report_num] == report) {
			return;
		}

		reports_[report_num] = report;

		scoped_sdl_surface& surf = reportSurfaces_[report_num];

		if(surf != NULL) {
			SDL_BlitSurface(surf,NULL,screen_.getSurface(),&rect);
			update_rect(rect);
		}

		//if the rectangle has just changed, assign the surface to it
		if(new_rect != rect || surf == NULL) {
			surf.assign(NULL);
			rect = new_rect;

			//if the rectangle is present, and we are blitting text, then
			//we need to backup the surface. (Images generally won't need backing
			//up unless they are transperant, but that is done later)
			if(rect.w > 0 && rect.h > 0) {
				surf.assign(get_surface_portion(screen_.getSurface(),rect));
				if(reportSurfaces_[report_num] == NULL) {
					std::cerr << "Could not backup background for report!\n";
				}
			}

			update_rect(rect);
		}

		tooltips::clear_tooltips(rect);

		SDL_Rect area = rect;

			int x = rect.x, y = rect.y;
		if(!report.empty()) {
			// Add prefix, postfix elements. Make sure that they get the same tooltip as the guys
			// around them.
			std::string str = item->prefix();
			if(str.empty() == false) {
				report.insert(report.begin(), reports::element(str,"",report.begin()->tooltip));
				}
			str = item->postfix();
			if(str.empty() == false) {
				report.push_back(reports::element(str,"",report.end()->tooltip));
			}

			// Loop through and display each report element
			size_t tallest = 0;
			for(reports::report::iterator i = report.begin(); i != report.end(); ++i) {
				if(i->text.empty() == false) {
					// Draw a text element
					area = font::draw_text(this,rect,item->font_size(),font::NORMAL_COLOUR,i->text,x,y);
					if(area.h > tallest) tallest = area.h;
					if(i->text[i->text.size() - 1] == '\n') {
						x = rect.x;
						y += tallest;
						tallest = 0;
			} else {
						x += area.w;
					}
				} else if(i->image.empty() == false) {
				//first back up the current area for later restoration
					//surf.assign(get_surface_portion(screen_.getSurface(),rect));

					// Draw an image element
					scoped_sdl_surface img(image::get_image(i->image,image::UNSCALED));

					if(img == NULL) {
						std::cerr << "could not find image for report: '" << i->image << "'\n";
						continue;
						}

					area.x = x;
					area.y = y;
					area.w = minimum<int>(rect.w + rect.x - x, img->w);
					area.h = minimum<int>(rect.h + rect.y - y, img->h);
					draw_image_for_report(img,surf,area);

					if(area.h > tallest) tallest = area.h;
					x += area.w;

				} else {
					// No text or image, skip this element
					continue;
					}
				if(i->tooltip.empty() == false) {
					tooltips::add_tooltip(area,i->tooltip);
				}
			}
		}
	} else {
		reportSurfaces_[report_num].assign(NULL);
	}
}

void display::draw_unit_details(int x, int y, const gamemap::location& loc,
         const unit& u, SDL_Rect& description_rect, int profilex, int profiley,
         SDL_Rect* clip_rect)
{
	if(teams_.empty())
		return;

	tooltips::clear_tooltips(description_rect);

	SDL_Rect clipRect = clip_rect != NULL ? *clip_rect : screen_area();

	const scoped_sdl_surface background(image::get_image(game_config::rightside_image,image::UNSCALED));
	const scoped_sdl_surface background_bot(image::get_image(game_config::rightside_image_bot,image::UNSCALED));

	if(background == NULL || background_bot == NULL)
		return;

	SDL_Surface* const screen = screen_.getSurface();

	if(description_rect.w > 0 && description_rect.x >= mapx()) {
		SDL_Rect srcrect = description_rect;
		srcrect.y -= background->h;
		srcrect.x -= mapx();

		SDL_BlitSurface(background_bot,&srcrect,screen,&description_rect);
		update_rect(description_rect);
	}

	std::string status = string_table["healthy"];
	if(map_.on_board(loc) &&
	   u.invisible(map_.underlying_terrain(map_[loc.x][loc.y]), 
			status_.get_time_of_day().lawful_bonus,loc,
			units_,teams_)) {
		status = font::GOOD_TEXT + string_table["invisible"];
	}

	if(u.has_flag("slowed")) {
		status = font::BAD_TEXT + string_table["slowed"];
	}

	if(u.has_flag("poisoned")) {
		status = font::BAD_TEXT + string_table["poisoned"];
	}

	std::stringstream details;
	details << font::LARGE_TEXT << u.description() << "\n"
	        << font::LARGE_TEXT << u.type().language_name()
			<< "\n-(" << string_table["level"] << " "
			<< u.type().level() << ")\n"
			<< status << "\n"
			<< translate_string(unit_type::alignment_description(u.type().alignment()))
			<< "\n"
			<< u.traits_description() << "\n";

	const std::vector<std::string>& abilities = u.type().abilities();
	for(std::vector<std::string>::const_iterator a = abilities.begin(); a != abilities.end(); ++a) {
		details << translate_string_default("ability_" + *a, *a) << "\n";
	}

	//display in green/white/red depending on hitpoints
	if(u.hitpoints() <= u.max_hitpoints()/3)
		details << font::BAD_TEXT;
	else if(u.hitpoints() > 2*(u.max_hitpoints()/3))
		details << font::GOOD_TEXT;

	details << string_table["hp"] << ": " << u.hitpoints()
			<< "/" << u.max_hitpoints() << "\n";
	
	if(u.type().advances_to().empty()) {
		details << string_table["xp"] << ": " << u.experience() << "/-";
	} else {
		//if killing a unit the same level as us would level us up,
		//then display in green
		if(u.max_experience() - u.experience() < game_config::kill_experience) {
			details << font::GOOD_TEXT;
		}

		details << string_table["xp"] << ": " << u.experience() << "/" << u.max_experience();
	}
	
	details << "\n"
			<< string_table["moves"] << ": " << u.movement_left() << "/"
			<< u.total_movement()
			<< "\n";

	const std::vector<attack_type>& attacks = u.attacks();
	for(std::vector<attack_type>::const_iterator at_it = attacks.begin();
	    at_it != attacks.end(); ++at_it) {

		const std::string& lang_weapon = string_table["weapon_name_" + at_it->name()];
		const std::string& lang_type = string_table["weapon_type_" + at_it->type()];
		const std::string& lang_special = string_table["weapon_special_" + at_it->special()];
		details << "\n"
				<< (lang_weapon.empty() ? at_it->name():lang_weapon) << " ("
				<< (lang_type.empty() ? at_it->type():lang_type) << ")\n"
				<< (lang_special.empty() ? at_it->special():lang_special)<<"\n"
				<< at_it->damage() << "-" << at_it->num_attacks() << " -- "
		        << (at_it->range() == attack_type::SHORT_RANGE ?
		            string_table["short_range"] :
					string_table["long_range"]);
	
		if(at_it->hexes() > 1) {
			details << " (" << at_it->hexes() << ")";
		}
					
		details << "\n\n";
	}

	//choose the font size based on how much room we have to play
	//with on the right-side panel
	const size_t font_size = this->y() >= 700 ? 13 : 10;

	description_rect = font::draw_text(this,clipRect,font_size,font::NORMAL_COLOUR,
	                                   details.str(),x,y);

	update_rect(description_rect);

	y += description_rect.h;

	const scoped_sdl_surface profile(image::get_image(u.type().image(),image::UNSCALED));

	if(profile == NULL)
		return;

	//blit the unit profile
	{
		const size_t profilew = 50;
		const size_t profileh = 50;
		SDL_Rect srcrect = { (profile->w-profilew)/2,(profile->h-profileh)/2,
		                     profilew,profileh };
		SDL_Rect dstrect = srcrect;
		dstrect.x = profilex;
		dstrect.y = profiley;
		SDL_BlitSurface(profile,&srcrect,video().getSurface(),&dstrect);

		update_rect(profilex,profiley,profilew,profileh);
	}
}

void display::draw_minimap(int x, int y, int w, int h)
{
	const scoped_sdl_surface surface(getMinimap(w,h));
	if(surface == NULL)
		return;

	SDL_Rect minimap_location = {x,y,w,h};

	clip_rect_setter clip_setter(video().getSurface(),minimap_location);

	SDL_Rect loc = minimap_location;
	SDL_BlitSurface(surface,NULL,video().getSurface(),&loc);

	for(unit_map::const_iterator u = units_.begin(); u != units_.end(); ++u) {
		if(fogged(u->first.x,u->first.y) || 
				(teams_[currentTeam_].is_enemy(u->second.side()) &&
				u->second.invisible(map_.underlying_terrain(map_[u->first.x][u->first.y]), 
				status_.get_time_of_day().lawful_bonus,u->first,
				units_,teams_)))
			continue;

		const int side = u->second.side();
		const SDL_Color& col = font::get_side_colour(side);
		const Uint16 mapped_col = SDL_MapRGB(video().getSurface()->format,col.r,col.g,col.b);
		SDL_Rect rect = {x + (u->first.x*w)/map_.x(),
		                 y + (u->first.y*h)/map_.y(),
						 w/map_.x(), h/map_.y() };
		SDL_FillRect(video().getSurface(),&rect,mapped_col);
	}

	const double xscaling = double(surface->w)/double(map_.x());
	const double yscaling = double(surface->h)/double(map_.y());

	const int xbox = static_cast<int>(xscaling*xpos_/(zoom_*0.75));
	const int ybox = static_cast<int>(yscaling*ypos_/zoom_);

	const int wbox = static_cast<int>(xscaling*map_area().w/(zoom_*0.75) - xscaling);
	const int hbox = static_cast<int>(yscaling*map_area().h/zoom_ - yscaling);

	const Uint16 boxcolour = Uint16(SDL_MapRGB(surface->format,0xFF,0xFF,0xFF));
	SDL_Surface* const screen = screen_.getSurface();

	gui::draw_rectangle(x+xbox,y+ybox,wbox,hbox,boxcolour,screen);

	update_rect(minimap_location);
}

void display::draw_terrain_palette(int x, int y, gamemap::TERRAIN selected)
{
	const int max_h = 35;

	SDL_Rect invalid_rect;
	invalid_rect.x = x;
	invalid_rect.y = y;
	invalid_rect.w = 0;

	SDL_Surface* const screen = screen_.getSurface();

	std::vector<gamemap::TERRAIN> terrains = map_.get_terrain_precedence();
	for(std::vector<gamemap::TERRAIN>::const_iterator i = terrains.begin();
	    i != terrains.end(); ++i) {
		const scoped_sdl_surface image(getTerrain(*i,image::SCALED,-1,-1));
		if(image == NULL) {
			std::cerr << "image for terrain '" << *i << "' not found\n";
			return;
		}

		if(x + image->w >= this->x() || y + image->h >= this->y()) {
			std::cerr << "terrain palette can't fit: " << x + image->w << " > " << this->x() << " or " << y+image->h << " > " << this->y() << "\n";
			return;
		}

		SDL_Rect dstrect;
		dstrect.x = x;
		dstrect.y = y;
		dstrect.w = image->w;
		dstrect.h = image->h;

		if(dstrect.h > max_h)
			dstrect.h = max_h;

		SDL_BlitSurface(image,NULL,screen,&dstrect);
		gui::draw_rectangle(x,y,image->w-1,max_h-1,
		                    *i == selected?0xF000:0,screen);

		y += max_h+2;

		if(image->w > invalid_rect.w)
			invalid_rect.w = image->w;
	}

	invalid_rect.h = y - invalid_rect.y;
	update_rect(invalid_rect);
}

gamemap::TERRAIN display::get_terrain_on(int palx, int paly, int x, int y)
{
	const int height = 37;
	if(y < paly || x < palx)
		return 0;

	const std::vector<gamemap::TERRAIN>& terrains=map_.get_terrain_precedence();
	if(static_cast<size_t>(y) > paly+terrains.size()*height)
		return 0;

	const size_t index = (y - paly)/height;
	if(index >= terrains.size())
		return 0;

	return terrains[index];
}

void display::draw_unit_on_tile(int x, int y, SDL_Surface* unit_image_override,
								double highlight_ratio, Uint32 blend_with)
{
	if(updatesLocked_)
		return;

	const gamemap::location loc(x,y);
	int xpos = int(get_location_x(loc));
	int ypos = int(get_location_y(loc));

	SDL_Rect clip_rect = map_area();

	if(xpos > clip_rect.x + clip_rect.w || ypos > clip_rect.y + clip_rect.h ||
	   xpos + static_cast<int>(zoom_) < clip_rect.x || ypos + static_cast<int>(zoom_) < clip_rect.y) {
		return;
	}

	SDL_Surface* const dst = screen_.getSurface();

	clip_rect_setter set_clip_rect(dst,clip_rect);

	double unit_energy = 0.0;

	Uint16 energy_colour = 0;

	const int max_energy = 80;
	double energy_size = 1.0;
	bool face_left = true;

	if(unit_image_override != NULL)
		sdl_add_ref(unit_image_override);

	scoped_sdl_surface unit_image(unit_image_override);
	scoped_sdl_surface energy_image(NULL);

	//see if there is a unit on this tile
	const unit_map::const_iterator it = units_.find(gamemap::location(x,y));
	if(it != units_.end() && (loc != hiddenUnit_ || !hideEnergy_)) {
		if(unit_image == NULL)
			unit_image.assign(image::get_image(it->second.image(),it->second.stone() ? image::GREYED : image::SCALED));

		if(unit_image == NULL) {
			return;
		}

		const int unit_move = it->second.movement_left();
		const int unit_total_move = it->second.total_movement();

		const std::string* energy_file = NULL;

		if(size_t(it->second.side()) != currentTeam_+1) {
			if(team_valid() &&
			   teams_[currentTeam_].is_enemy(it->second.side())) {
				energy_file = &game_config::enemy_energy_image;
			} else {
				energy_file = &game_config::ally_energy_image;
			}
		} else {
			if(activeTeam_ == currentTeam_ && unit_move == unit_total_move) {
				energy_file = &game_config::unmoved_energy_image;
			} else if(activeTeam_ == currentTeam_ && unit_can_move(loc,units_,map_,teams_)) {
				energy_file = &game_config::partmoved_energy_image;
			} else {
				energy_file = &game_config::moved_energy_image;
			}
		}

		assert(energy_file != NULL);
		if(energy_file == NULL) {
			std::cerr << "energy file is NULL\n";
			return;
		}

		energy_image.assign(image::get_image(*energy_file,image::SCALED,image::NO_ADJUST_COLOUR));

		if(energy_image.get() == NULL) {
			std::cerr << "failed to get energy image: '" << *energy_file << "'\n";
			return;
		}

		unit_energy = minimum<double>(1.0,double(it->second.hitpoints()) / double(it->second.max_hitpoints()));

		if(highlight_ratio == 1.0)
			highlight_ratio = it->second.alpha();

		if(it->second.invisible(map_.underlying_terrain(map_[x][y]), 
					status_.get_time_of_day().lawful_bonus,loc,
					units_,teams_) &&
		   highlight_ratio > 0.5) {
			highlight_ratio = 0.5;
		}

		if(loc == selectedHex_ && highlight_ratio == 1.0) {
			highlight_ratio = 1.5;
			blend_with = rgb(255,255,255);
		}

		{
			int er = 0;
			int eg = 0;
			int eb = 0;
			if(unit_energy < 0.33) {
				er = 200;
			} else if(unit_energy < 0.66) {
				er = 200;
				eg = 200;
			} else {
				eg = 200;
			}

			energy_colour = ::SDL_MapRGB(screen_.getSurface()->format,er,eg,eb);
		}

		if(it->second.max_hitpoints() < max_energy) {
			energy_size = double(it->second.max_hitpoints())/double(max_energy);
		}

		face_left = it->second.facing_left();
	}

	if(deadUnit_ == gamemap::location(x,y)) {
		highlight_ratio = deadAmount_;
	}

	if(unit_image == NULL || energy_image == NULL || fogged(x,y) ||
			(teams_[currentTeam_].is_enemy(it->second.side()) && 
			it->second.invisible(map_.underlying_terrain(map_[x][y]), 
					status_.get_time_of_day().lawful_bonus,loc,
					units_,teams_))) {
		return;
	}

	const gamemap::TERRAIN terrain = map_.get_terrain(loc);
	const int height_adjust = it->second.is_flying() ? 0 : int(map_.get_terrain_info(terrain).unit_height_adjust()*(zoom_/DefaultZoom));
	const double submerge = it->second.is_flying() ? 0.0 : map_.get_terrain_info(terrain).unit_submerge();

	if(loc == advancingUnit_ && it != units_.end()) {
		//the unit is advancing - set the advancing colour to white if it's a
		//non-chaotic unit, otherwise black
		blend_with = it->second.type().alignment() == unit_type::CHAOTIC ?
		                                        rgb(16,16,16) : rgb(255,255,255);
		highlight_ratio = advancingAmount_;
	} else if(it->second.poisoned() && highlight_ratio == 1.0) {
		//the unit is poisoned - draw with a green hue
		blend_with = rgb(0,255,0);
		highlight_ratio = 0.75;
	}

	const bool energy_uses_alpha = highlight_ratio < 1.0 && blend_with == 0;

	if(loc != hiddenUnit_) {
		scoped_sdl_surface ellipse_front(NULL);
		scoped_sdl_surface ellipse_back(NULL);

		if(preferences::show_side_colours()) {
			char buf[50];
			sprintf(buf,"misc/ellipse-%d-top.png",it->second.side());
			ellipse_back.assign(image::get_image(buf));
			sprintf(buf,"misc/ellipse-%d-bottom.png",it->second.side());
			ellipse_front.assign(image::get_image(buf));
		}

		draw_unit(xpos,ypos - height_adjust,unit_image,face_left,false,
		          highlight_ratio,blend_with,submerge,ellipse_back,ellipse_front);
	}

	const SDL_Rect& energy_bar_loc = calculate_energy_bar();
	double total_energy = double(energy_bar_loc.h);

	const int skip_energy_rows = int(total_energy*(1.0-energy_size));
	total_energy -= double(skip_energy_rows);

	const int lost_energy = int((1.0-unit_energy)*total_energy);
	const int show_energy_after = energy_bar_loc.y + lost_energy;

	if(energy_uses_alpha) {
		energy_image.assign(adjust_surface_alpha(energy_image,highlight_ratio));
	}

	SDL_Rect first_energy = {0,0,energy_image->w,energy_bar_loc.y};
	SDL_Rect second_energy = {0,energy_bar_loc.y+skip_energy_rows,energy_image->w,0};
	second_energy.h = energy_image->w - second_energy.y;

	blit_surface(xpos,ypos,energy_image,&first_energy,&clip_rect);
	blit_surface(xpos,ypos+first_energy.h,energy_image,&second_energy,&clip_rect);

	if(skip_energy_rows + lost_energy < energy_bar_loc.h) {
		SDL_Rect filled_energy_area = { xpos + energy_bar_loc.x, ypos+show_energy_after,
		          energy_bar_loc.w, energy_bar_loc.h - skip_energy_rows - lost_energy };
		SDL_FillRect(dst,&filled_energy_area,energy_colour);
	}
}

void display::draw_tile_adjacent(int x, int y, image::TYPE image_type, ADJACENT_TERRAIN_TYPE type)
{
	const gamemap::location loc(x,y);
	int xpos = int(get_location_x(loc));
	int ypos = int(get_location_y(loc));

	SDL_Rect clip_rect = map_area();

	if(xpos > clip_rect.x + clip_rect.w || ypos > clip_rect.y + clip_rect.h ||
	   xpos + static_cast<int>(zoom_) < clip_rect.x || ypos + static_cast<int>(zoom_) < clip_rect.y) {
		return;
	}

	SDL_Surface* const dst = screen_.getSurface();

	clip_rect_setter set_clip_rect(dst,clip_rect);
	
	const std::vector<shared_sdl_surface>& adj = getAdjacentTerrain(x,y,image_type,type);

	std::vector<shared_sdl_surface>::const_iterator i;
	for(i = adj.begin(); i != adj.end(); ++i) {
		SDL_Rect dstrect = { xpos, ypos, 0, 0 };
		SDL_BlitSurface(*i,NULL,dst,&dstrect);
	}

	const std::vector<shared_sdl_surface>& built = getBuiltTerrain(x,y,image_type,type);

	for(i = built.begin(); i != built.end(); ++i) {
		SDL_Rect dstrect = { xpos, ypos, 0, 0 };
		SDL_BlitSurface(*i,NULL,dst,&dstrect);
	}
}

void display::draw_tile(int x, int y, SDL_Surface* unit_image, double alpha, Uint32 blend_to)
{
	if(updatesLocked_)
		return;
	
	const gamemap::location loc(x,y);
	int xpos = int(get_location_x(loc));
	int ypos = int(get_location_y(loc));

	SDL_Rect clip_rect = map_area();

	if(xpos > clip_rect.x + clip_rect.w || ypos > clip_rect.y + clip_rect.h ||
	   xpos + static_cast<int>(zoom_) < clip_rect.x || ypos + static_cast<int>(zoom_) < clip_rect.y) {
		return;
	}

	SDL_Surface* const dst = screen_.getSurface();

	clip_rect_setter set_clip_rect(dst,clip_rect);

	const bool is_shrouded = shrouded(x,y);
	gamemap::TERRAIN terrain = gamemap::VOID_TERRAIN;

	if(!is_shrouded) {
		terrain = map_.get_terrain(loc);
	}

	image::TYPE image_type = image::SCALED;

	//find if this tile should be greyed
	if(pathsList_ != NULL && pathsList_->routes.find(gamemap::location(x,y)) ==
					         pathsList_->routes.end()) {
		image_type = image::GREYED;
	}

	unit_map::iterator un = find_visible_unit(units_, loc, map_,
		status_.get_time_of_day().lawful_bonus,teams_,teams_[currentTeam_]);

	if(loc == mouseoverHex_ && map_.on_board(mouseoverHex_) ||
	   loc == selectedHex_ && (un != units_.end())) {
		image_type = image::BRIGHTENED;
	}

	scoped_sdl_surface surface(getTerrain(terrain,image_type,x,y));

	if(surface == NULL) {
		std::cerr << "Could not get terrain surface\n";
		return;
	}

	update_rect(xpos,ypos,surface->w,surface->h);

	//note that dstrect can be changed by SDL_BlitSurface and so a new instance should be
	//initialized to pass to each call to SDL_BlitSurface
	SDL_Rect dstrect = { xpos, ypos, 0, 0 };
	SDL_BlitSurface(surface,NULL,dst,&dstrect);

	if(!is_shrouded) {
		scoped_sdl_surface flag(getFlag(terrain,x,y));
		if(flag != NULL) {
			SDL_Rect dstrect = { xpos, ypos, 0, 0 };
			SDL_BlitSurface(flag,NULL,dst,&dstrect);
		}

		draw_tile_adjacent(x,y,image_type,ADJACENT_BACKGROUND);

		typedef std::multimap<gamemap::location,std::string>::const_iterator Itor;

		for(std::pair<Itor,Itor> overlays = overlays_.equal_range(loc);
			overlays.first != overlays.second; ++overlays.first) {

			scoped_sdl_surface overlay_surface(image::get_image(overlays.first->second));
			
			if(overlay_surface != NULL) {
				SDL_Rect dstrect = { xpos, ypos, 0, 0 };
				SDL_BlitSurface(overlay_surface,NULL,dst,&dstrect);
			}
		}
	}

	if(!is_shrouded) {
		draw_footstep(loc,xpos,ypos);
	}

	if(fogged(x,y)) {
		const scoped_sdl_surface fog_surface(image::get_image("terrain/fog.png"));
		if(fog_surface != NULL) {
			SDL_Rect dstrect = { xpos, ypos, 0, 0 };
			SDL_BlitSurface(fog_surface,NULL,dst,&dstrect);
		}
	}

	draw_unit_on_tile(x,y,unit_image,alpha,blend_to);

	if(!shrouded(x,y)) {
		draw_tile_adjacent(x,y,image_type,ADJACENT_FOREGROUND);
	}

	if(grid_) {
		scoped_sdl_surface grid_surface(image::get_image("terrain/grid.png"));
		if(grid_surface != NULL) {
			SDL_Rect dstrect = { xpos, ypos, 0, 0 };
			SDL_BlitSurface(grid_surface,NULL,dst,&dstrect);
		}
	}

	if(game_config::debug && debugHighlights_.count(gamemap::location(x,y))) {
		const scoped_sdl_surface cross(image::get_image(game_config::cross_image));
		if(cross != NULL)
			draw_unit(xpos,ypos,cross,false,false,
			          debugHighlights_[loc],0);
	}
}

void display::draw_footstep(const gamemap::location& loc, int xloc, int yloc)
{
	std::vector<gamemap::location>::const_iterator i =
	         std::find(route_.steps.begin(),route_.steps.end(),loc);

	if(i == route_.steps.begin() || i == route_.steps.end())
		return;

	const bool show_time = (i+1 == route_.steps.end());

	const bool left_foot = is_even(i - route_.steps.begin());

	//generally we want the footsteps facing toward the direction they're going
	//to go next.
	//if we're on the last step, then we want them facing according to where
	//they came from, so we move i back by one
	if(i+1 == route_.steps.end() && i != route_.steps.begin())
		--i;

	gamemap::location::DIRECTION direction = gamemap::location::NORTH;

	if(i+1 != route_.steps.end()) {
		for(int n = 0; n != 6; ++n) {
			direction = gamemap::location::DIRECTION(n);
			if(i->get_direction(direction) == *(i+1)) {
				break;
			}
		}
	}

	static const std::string left_nw(game_config::foot_left_nw);
	static const std::string left_n(game_config::foot_left_n);
	static const std::string right_nw(game_config::foot_right_nw);
	static const std::string right_n(game_config::foot_right_n);

	const std::string* image_str = &left_nw;

	if(left_foot) {
		if(direction == gamemap::location::NORTH ||
		   direction == gamemap::location::SOUTH) {
			image_str = &left_n;
		} else {
			image_str = &left_nw;
		}
	} else {
		if(direction == gamemap::location::NORTH ||
		   direction == gamemap::location::SOUTH) {
			image_str = &right_n;
		} else {
			image_str = &right_nw;
		}
	}

	const scoped_sdl_surface image(image::get_image(*image_str));
	if(image == NULL) {
		std::cerr << "Could not find image: " << *image_str << "\n";
		return;
	}

	const bool hflip = !(direction > gamemap::location::NORTH &&
	                     direction <= gamemap::location::SOUTH);
	const bool vflip = (direction >= gamemap::location::SOUTH_EAST &&
	                    direction <= gamemap::location::SOUTH_WEST);

	draw_unit(xloc,yloc,image,hflip,vflip,0.5);

	if(show_time && route_.move_left > 0 && route_.move_left < 10) {
		//draw number in yellow if terrain is light, else draw in black
		gamemap::TERRAIN terrain = map_.get_terrain(loc);
		const bool tile_is_light = map_.get_terrain_info(terrain).is_light();
		SDL_Color text_colour = tile_is_light ? font::DARK_COLOUR : font::YELLOW_COLOUR;

		const SDL_Rect& rect = map_area();
		std::string str(1,'x');
		str[0] = '1' + route_.move_left;
		const SDL_Rect& text_area = font::draw_text(NULL,rect,18,text_colour,str,0,0);
		const int x = xloc + int(zoom_/2.0) - text_area.w/2;
		const int y = yloc + int(zoom_/2.0) - text_area.h/2;
		font::draw_text(this,rect,18,text_colour,str,x,y);
	}
}

namespace {
const std::string& get_direction(size_t n)
{
	const static std::string dirs[6] = {"-n","-ne","-se","-s","-sw","-nw"};
	return dirs[n >= sizeof(dirs)/sizeof(*dirs) ? 0 : n];
}

bool angle_is_northern(size_t n)
{
	const static bool results[6] = {true,false,false,false,false,true};
	return results[n >= sizeof(results)/sizeof(*results) ? 0 : n];
}

const std::string& get_angle_direction(size_t n)
{
	const static std::string dirs[6] = {"-ne","-e","-se","-sw","-w","-nw"};
	return dirs[n >= sizeof(dirs)/sizeof(*dirs) ? 0 : n];
}

}

std::vector<shared_sdl_surface> display::getAdjacentTerrain(int x, int y, image::TYPE image_type, ADJACENT_TERRAIN_TYPE terrain_type)
{
	std::vector<shared_sdl_surface> res;
	gamemap::location loc(x,y);

	const gamemap::TERRAIN current_terrain = map_.get_terrain(loc);

	gamemap::location adjacent[6];
	get_adjacent_tiles(loc,adjacent);
	int tiles[6];
	for(int i = 0; i != 6; ++i) {
		if(terrain_type == ADJACENT_FOREGROUND && shrouded(adjacent[i].x,adjacent[i].y))
			tiles[i] = gamemap::VOID_TERRAIN;
		else if(terrain_type == ADJACENT_FOREGROUND && !fogged(x,y) && fogged(adjacent[i].x,adjacent[i].y))
			tiles[i] = gamemap::FOGGED;
		else
			tiles[i] = map_.get_terrain(adjacent[i]);
	}

	static std::vector<gamemap::TERRAIN> fog_shroud;
	if(fog_shroud.empty()) {
		fog_shroud.push_back(gamemap::VOID_TERRAIN);
		fog_shroud.push_back(gamemap::FOGGED);
	}

	const std::vector<gamemap::TERRAIN>& precedence = (terrain_type == ADJACENT_BACKGROUND) ?
	                                             map_.get_terrain_precedence() : fog_shroud;
	std::vector<gamemap::TERRAIN>::const_iterator terrain =
	       std::find(precedence.begin(),precedence.end(),current_terrain);

	if(terrain == precedence.end()) {
		terrain = precedence.begin();
	} else {
		++terrain;
		while(terrain != precedence.end() &&
		      map_.get_terrain_info(*terrain).equal_precedence()) {
			++terrain;
		}
	}

	for(; terrain != precedence.end(); ++terrain){
		//find somewhere that doesn't have overlap to use as a starting point
		int start;
		for(start = 0; start != 6; ++start) {
			if(tiles[start] != *terrain)
				break;
		}

		if(start == 6) {
			start = 0;
		}

		//find all the directions overlap occurs from
		for(int i = (start+1)%6, n = 0; i != start && n != 6; ++n) {
			if(tiles[i] == *terrain) {
				shared_sdl_surface surface(NULL);
				std::ostringstream stream;
				for(int n = 0; *terrain == tiles[i] && n != 6; i = (i+1)%6, ++n) {

					stream << get_direction(i);
					const shared_sdl_surface new_surface(getTerrain(
					                    *terrain,image_type,x,y,stream.str()));

					if(new_surface == NULL) {
						//if we don't have any surface at all,
						//then move onto the next overlapped area
						if(surface == NULL)
							i = (i+1)%6;
						break;
					}

					surface = new_surface;
				}

				if(surface != NULL)
					res.push_back(surface);
			} else {
				i = (i+1)%6;
			}
		}
	}

	return res;
}

std::vector<shared_sdl_surface> display::getBuiltTerrain(int x, int y, image::TYPE image_type, ADJACENT_TERRAIN_TYPE terrain_type)
{
	std::vector<shared_sdl_surface> res;
	gamemap::location loc(x,y);
	
	// If the current tile is a castle tile, or if any adjacent tile is, this tile will have
	// some castle-wall adjustable decorations.
	// For now, the type of tiles (castle, !castle) is built-in.
	// For now, the type of tiles (castle, !castle) is built-in.

	gamemap::location adjacent[6];
	get_adjacent_tiles(loc,adjacent);
	
	signed char angle_type;
	bool ti, tj, to;
	// TODO: change this to something like map_.is_castle(blah) to support
	// keeps, elven/orcish castles, etc.
	gamemap::TERRAIN terrain = gamemap::CASTLE;
	to = map_.is_built(loc);

 	// Adjacent terrains use 6 directions. From direction 0 to direction 5, these are
 	// n, ne, se, s, sw, nw . To build castles, we will introduce corners. From corner
 	// 0 to corner 5, those are ne, e, se, sw, w, nw ; corner i is the angle between
 	// side i and side (i+1) (modulus 6).
 	// On each corner, the boundary between a castle-tile and a non-castle tile may be
 	// of 6 different types, each one corresponding to a corner. Those are shown below:
 	//     0 (ne)   1 (e)   2 (se)   3 (sw)   4 (w)  5 (nw)
 	//     _        \       _/       \_       /      _
 	//      \       /                         \     /
 	// Additionaly, each angle may be a convex (-i), or a concave (-e) angle.
 	// Castles are built for angles. Castle tiles names start with castle-<angle>-<conc>-
 	// to represent the angle they are on.
 	// Finally, each angle is composed of 3 tiles. Those are named according to the corner
 	// this angle would be on, for this tile. That is:
 	//
 	// (even angles) (odd angles)
 	//  se  /            \  sw
 	//  ___/ w        e   \___
    //     \              /
    //  ne  \            /  ne
 	//
 	// Castle tiles complete names are: castle-<angle>-<conc>-<tile>.png
	for(int i = 0; i != 6; ++i) {
		int j = i+1;
		if(j == 6)
			j = 0;
		
		ti = map_.is_built(adjacent[i]);
		tj = map_.is_built(adjacent[j]);

 		// Determine the angle type, according to the built adjacent tiles. 
 		// (This is the tricky part).
 		// If one of the three tiles is different, the direction at which the current corner
 		// is, relatively to this different tile, is the angle direction. If the different tile
 		// is the current tile, the angle direction is the direction corresponding to the current
 		// corner. If the different tile is the first of the adjacent tiles, the angle direction 
 		// is "the direction corresponding to the current corner, according to the first adjacent
 		// tile", which is the direction corresponding to the current corner, rotated 2pi/3 (pheeew).
 		// Hence the modulus. QED. (the same for the other adjacent tile)
 		// (trust me, it is way more easy to understand when drawn :) )
		if ((ti != to) && (tj != to)) {
			angle_type = i;
		} else if ((ti == to) && (tj != to)) {
			angle_type = (i+4) % 6;
		} else if ((ti != to) && (tj == to)) {
			angle_type = (i+2) % 6;
		} else {
			angle_type = -1;
		}

		if(angle_type == -1) {
			continue;
		}

 		// Count the number of built tiles between the current tile, and the two
 		// tiles adjacent to the current corner. If only on of those tiles is built,
 		// the angle is convex. If two are built, the angle is concave. If 0 or 3 are 
 		// built, there is no wall there.
		int ncastles = (to?1:0) + (ti?1:0) + (tj?1:0);

		const bool angle_northern = angle_is_northern(i);
		if(angle_northern && terrain_type == ADJACENT_FOREGROUND ||
			!angle_northern && terrain_type == ADJACENT_BACKGROUND) {
			continue;
		}

		static const std::string exterior = "-e", interior = "-i";
		std::ostringstream stream;
		stream << get_angle_direction(angle_type) << 
			(ncastles == 2? exterior : interior) << get_angle_direction(i);
		const shared_sdl_surface surface(getTerrain(terrain,
							    image_type,x,y,stream.str()));
		if(surface != NULL)
			res.push_back(surface);
	}

	return res;
}

SDL_Surface* display::getTerrain(gamemap::TERRAIN terrain,image::TYPE image_type,
                                 int x, int y, const std::string& direction)
{
	std::string image = "terrain/" + (direction.empty() ?
	                           map_.get_terrain_info(terrain).image(x,y) :
	                           map_.get_terrain_info(terrain).adjacent_image());

	const time_of_day& tod = status_.get_time_of_day();
	const time_of_day& tod_at = timeofday_at(status_,units_,gamemap::location(x,y));

	//see if there is a time-of-day specific version of this image
	if(direction == "") {
		
		const std::string tod_image = image + "-" + tod.id + ".png";
		SDL_Surface* const im = image::get_image(tod_image,image_type);

		if(im != NULL) {
			return im;
		}
	}

	image += direction + ".png";

	SDL_Surface* im = image::get_image(image,image_type);
	if(im == NULL && direction.empty()) {
		im = image::get_image("terrain/" +
		        map_.get_terrain_info(terrain).default_image() + ".png");
	}

	//see if this tile is illuminated to a different colour than it'd
	//normally be displayed as
	const int radj = tod_at.red - tod.red;
	const int gadj = tod_at.green - tod.green;
	const int badj = tod_at.blue - tod.blue;

	if((radj|gadj|badj) != 0 && im != NULL) {
		const scoped_sdl_surface backup(im);
		std::cerr << "adjusting surface colour " << radj << "," << gadj << "," << badj << "\n";
		im = adjust_surface_colour(im,radj,gadj,badj);
		std::cerr << "done adjust...\n";
		if(im == NULL)
			std::cerr << "could not adjust surface..\n";
	}

	return im;
}

SDL_Surface* display::getFlag(gamemap::TERRAIN terrain, int x, int y)
{
	const bool village = map_.is_village(terrain);
	if(!village)
		return NULL;

	const gamemap::location loc(x,y);

	for(size_t i = 0; i != teams_.size(); ++i) {
		if(teams_[i].owns_tower(loc) && (!fogged(x,y) || !shrouded(x,y) && !teams_[currentTeam_].is_enemy(i+1))) {
			char buf[50];
			sprintf(buf,"terrain/flag-team%d.png",i+1);
			return image::get_image(buf);
		}
	}

	return NULL;
}

void display::blit_surface(int x, int y, SDL_Surface* surface, SDL_Rect* srcrect, SDL_Rect* clip_rect)
{
	SDL_Surface* const target = video().getSurface();

	SDL_Rect clip;
	if(clip_rect == NULL) {
		clip = screen_area();
		clip_rect = &clip;
	}

	SDL_Rect src;
	if(srcrect == NULL) {
		src.x = 0;
		src.y = 0;
		src.w = surface->w;
		src.h = surface->h;
		srcrect = &src;
	}

	if(x + srcrect->w < clip_rect->x) {
		return;
	}

	if(y + srcrect->h < clip_rect->y) {
		return;
	}

	if(x > clip_rect->x + clip_rect->w) {
		return;
	}

	if(y > clip_rect->y + clip_rect->h) {
		return;
	}

	if(x < clip_rect->x) {
		const int diff = clip_rect->x - x;
		srcrect->x += diff;
		srcrect->w -= diff;
		x = clip_rect->x;
	}

	if(y < clip_rect->y) {
		const int diff = clip_rect->y - y;
		srcrect->y += diff;
		srcrect->h -= diff;
		y = clip_rect->y;
	}

	if(x + srcrect->w > clip_rect->x + clip_rect->w) {
		srcrect->w = clip_rect->x + clip_rect->w - x;
	}

	if(y + srcrect->h > clip_rect->y + clip_rect->h) {
		srcrect->h = clip_rect->y + clip_rect->h - y;
	}

	SDL_Rect dstrect = {x,y,0,0};
	SDL_BlitSurface(surface,srcrect,target,&dstrect);
}

SDL_Surface* display::getMinimap(int w, int h)
{
	if(minimap_ == NULL) {
		std::cerr << "regetting minimap\n";
		minimap_ = image::getMinimap(w,h,map_,
				status_.get_time_of_day().lawful_bonus,
				team_valid() ? &teams_[currentTeam_] : NULL);
		std::cerr << "done regetting minimap\n";
	}

	sdl_add_ref(minimap_);
	return minimap_;
}

void display::set_paths(const paths* paths_list)
{
	pathsList_ = paths_list;
	invalidate_all();
}

void display::invalidate_route()
{
	for(std::vector<gamemap::location>::const_iterator i = route_.steps.begin();
	    i != route_.steps.end(); ++i) {
		invalidate(*i);
	}
}

void display::set_route(const paths::route* route)
{
	invalidate_route();

	if(route != NULL)
		route_ = *route;
	else
		route_.steps.clear();

	invalidate_route();
}

void display::move_unit(const std::vector<gamemap::location>& path, unit& u)
{
	for(size_t i = 0; i+1 < path.size(); ++i) {
		if(path[i+1].x > path[i].x) {
			u.set_facing_left(true);
		} else if(path[i+1].x < path[i].x) {
			u.set_facing_left(false);
		}

		//remove the footstep we are on.
		const std::vector<gamemap::location>::iterator it = std::find(route_.steps.begin(),route_.steps.end(),path[i]);
		if(it != route_.steps.end())
			route_.steps.erase(it);

		move_unit_between(path[i],path[i+1],u);
	}

	//make sure the entire path is cleaned properly
	for(std::vector<gamemap::location>::const_iterator it = path.begin();
	    it != path.end(); ++it) {
		draw_tile(it->x,it->y);
	}
}

void display::float_label(const gamemap::location& loc, const std::string& text,
						  int red, int green, int blue)
{
	if(preferences::show_floating_labels() == false) {
		return;
	}

	const SDL_Color colour = {red,green,blue,255};
	font::add_floating_label(text,24,colour,get_location_x(loc)+zoom_*0.5,get_location_y(loc),
	                         0,-2,60,screen_area());
}

bool display::unit_attack_ranged(const gamemap::location& a,
                                 const gamemap::location& b, int damage,
                                 const attack_type& attack)
{
	const bool hide = update_locked() || fogged(a.x,a.y) && fogged(b.x,b.y)
	                  || preferences::show_combat() == false;

	const unit_map::iterator att = units_.find(a);
	const unit_map::iterator def = units_.find(b);

	def->second.set_defending(true,attack_type::LONG_RANGE);

	//the missile frames are based around the time when the missile impacts.
	//the 'real' frames are based around the time when the missile launches.
	const int first_missile = minimum<int>(-100,
	                        attack.get_first_frame(attack_type::MISSILE_FRAME));
	const int last_missile = attack.get_last_frame(attack_type::MISSILE_FRAME);

	const int real_last_missile = last_missile - first_missile;
	const int missile_impact = -first_missile;

	const int time_resolution = 20;
	const int acceleration = turbo() ? 5:1;

	const std::vector<attack_type::sfx>& sounds = attack.sound_effects();
	std::vector<attack_type::sfx>::const_iterator sfx_it = sounds.begin();

	const std::string& hit_sound = def->second.type().get_hit_sound();
	bool played_hit_sound = (hit_sound == "" || hit_sound == "null");
	const int play_hit_sound_at = 0;

	const bool hits = damage > 0;
	const int begin_at = attack.get_first_frame();
	const int end_at   = maximum((damage+1)*time_resolution+missile_impact,
					       maximum(attack.get_last_frame(),real_last_missile));

	const double xsrc = get_location_x(a);
	const double ysrc = get_location_y(a);
	const double xdst = get_location_x(b);
	const double ydst = get_location_y(b);

	gamemap::location update_tiles[6];
	get_adjacent_tiles(a,update_tiles);

	const bool vflip = b.y > a.y || b.y == a.y && is_even(a.x);
	const bool hflip = b.x < a.x;
	const attack_type::FRAME_DIRECTION dir =
	         (a.x == b.x) ? attack_type::VERTICAL:attack_type::DIAGONAL;

	bool dead = false;
	const int drain_speed = 1*acceleration;

	int flash_num = 0;

	int ticks = SDL_GetTicks();

	for(int i = begin_at; i < end_at; i += time_resolution*acceleration) {
		events::pump();

		//this is a while instead of an if, because there might be multiple
		//sounds playing simultaneously or close together
		while(!hide && sfx_it != sounds.end() && i >= sfx_it->time) {
			const std::string& sfx = hits ? sfx_it->on_hit : sfx_it->on_miss;
			if(sfx.empty() == false) {
				sound::play_sound(hits ? sfx_it->on_hit : sfx_it->on_miss);
			}

			++sfx_it;
		}

		if(!hide && hits && !played_hit_sound && i >= play_hit_sound_at) {
			sound::play_sound(hit_sound);
			played_hit_sound = true;
		}

		const std::string* unit_image = attack.get_frame(i);

		if(unit_image == NULL) {
			unit_image =
			       &att->second.type().image_fighting(attack_type::LONG_RANGE);
		}

		if(!hide) {
			const scoped_sdl_surface image((unit_image == NULL) ? NULL : image::get_image(*unit_image));
			draw_tile(a.x,a.y,image);
		}

		if(damage > 0 && i == missile_impact) {
			char buf[50];
			sprintf(buf,"%d",damage);
			float_label(b,buf,255,0,0);
		}

		Uint32 defensive_colour = 0;
		double defensive_alpha = 1.0;

		if(damage > 0 && i >= missile_impact) {
			if(def->second.gets_hit(minimum<int>(drain_speed,damage))) {
				dead = true;
				damage = 0;
			} else {
				damage -= drain_speed;
			}

			if(flash_num == 0 || flash_num == 2) {
				defensive_alpha = 0.0;
				defensive_colour = rgb(200,0,0);
			}

			++flash_num;
		}

		for(int j = 0; j != 6; ++j) {
			if(update_tiles[j] != b) {
				draw_tile(update_tiles[j].x,update_tiles[j].y);
			}
		}

		draw_tile(b.x,b.y,NULL,defensive_alpha,defensive_colour);

		if(i >= 0 && i < real_last_missile && !hide) {
			const int missile_frame = i + first_missile;

			const std::string* missile_image
			         = attack.get_frame(missile_frame,NULL,
			                            attack_type::MISSILE_FRAME,dir);

			static const std::string default_missile(game_config::missile_n_image);
			static const std::string default_diag_missile(game_config::missile_ne_image);
			if(missile_image == NULL) {
				if(dir == attack_type::VERTICAL)
					missile_image = &default_missile;
				else
					missile_image = &default_diag_missile;
			}

			const scoped_sdl_surface img(image::get_image(*missile_image));
			if(img != NULL) {
				double pos = double(missile_impact - i)/double(missile_impact);
				if(pos < 0.0)
					pos = 0.0;
				const int xpos = int(xsrc*pos + xdst*(1.0-pos));
				const int ypos = int(ysrc*pos + ydst*(1.0-pos));

				draw_unit(xpos,ypos,img,!hflip,vflip);
			}
		}

		const int wait_time = ticks + time_resolution - SDL_GetTicks();
		if(wait_time > 0 && !hide)
			SDL_Delay(wait_time);

		ticks = SDL_GetTicks();

		update_display();
	}

	def->second.set_defending(false);

	draw_tile(a.x,a.y);
	draw_tile(b.x,b.y);

	if(dead) {
		unit_die(b);
	}

	return dead;
}

bool display::unit_attack(const gamemap::location& a,
                          const gamemap::location& b, int damage,
						  const attack_type& attack)
{
	const bool hide = update_locked() || fogged(a.x,a.y) && fogged(b.x,b.y)
	                  || preferences::show_combat() == false;

	if(!hide) {
		const double side_threshhold = 80.0;

		double xloc = get_location_x(a);
		double yloc = get_location_y(a);

		//we try to scroll the map if the unit is at the edge.
		//keep track of the old position, and if the map moves at all,
		//then recenter it on the unit
		double oldxpos = xpos_;
		double oldypos = ypos_;
		if(xloc < map_area().x + side_threshhold) {
			scroll(xloc - side_threshhold - map_area().x,0.0);
		}

		if(yloc < map_area().y + side_threshhold) {
			scroll(0.0,yloc - side_threshhold - map_area().y);
		}

		if(xloc + zoom_ > map_area().x + map_area().w - side_threshhold) {
			scroll(((xloc + zoom_) -
			        (map_area().x + map_area().w - side_threshhold)),0.0);
		}

		if(yloc + zoom_ > map_area().y + map_area().h - side_threshhold) {
			scroll(0.0,((yloc + zoom_) -
			        (map_area().y + map_area().h - side_threshhold)));
		}

		if(oldxpos != xpos_ || oldypos != ypos_) {
			scroll_to_tile(a.x,a.y,WARP);
		}
	}

	log_scope("unit_attack");
	invalidate_all();
	draw(true,true);

	const unit_map::iterator att = units_.find(a);
	assert(att != units_.end());

	unit& attacker = att->second;

	const unit_map::iterator def = units_.find(b);
	assert(def != units_.end());

	if(b.x > a.x) {
		att->second.set_facing_left(true);
		def->second.set_facing_left(false);
	} else if(b.x < a.x) {
		att->second.set_facing_left(false);
		def->second.set_facing_left(true);
	}

	if(attack.range() == attack_type::LONG_RANGE) {
		return unit_attack_ranged(a,b,damage,attack);
	}

	const bool hits = damage > 0;
	const std::vector<attack_type::sfx>& sounds = attack.sound_effects();
	std::vector<attack_type::sfx>::const_iterator sfx_it = sounds.begin();

	const std::string& hit_sound = def->second.type().get_hit_sound();
	bool played_hit_sound = (hit_sound == "" || hit_sound == "null");
	const int play_hit_sound_at = 0;

	const int time_resolution = 20;
	const int acceleration = turbo() ? 5 : 1;

	def->second.set_defending(true,attack_type::SHORT_RANGE);

	const int begin_at = minimum<int>(-200,attack.get_first_frame());
	const int end_at = maximum<int>((damage+1)*time_resolution,
	                                       maximum<int>(200,
	                                         attack.get_last_frame()));

	const double xsrc = get_location_x(a);
	const double ysrc = get_location_y(a);
	const double xdst = get_location_x(b)*0.6 + xsrc*0.4;
	const double ydst = get_location_y(b)*0.6 + ysrc*0.4;

	gamemap::location update_tiles[6];
	get_adjacent_tiles(b,update_tiles);

	bool dead = false;
	const int drain_speed = 1*acceleration;

	int flash_num = 0;

	int ticks = SDL_GetTicks();

	hiddenUnit_ = a;

	const gamemap::TERRAIN src_terrain = map_.get_terrain(a);
	const gamemap::TERRAIN dst_terrain = map_.get_terrain(b);

	const double src_height_adjust = attacker.is_flying() ? 0 : map_.get_terrain_info(src_terrain).unit_height_adjust() * (zoom_/DefaultZoom);
	const double dst_height_adjust = attacker.is_flying() ? 0 : map_.get_terrain_info(dst_terrain).unit_height_adjust() * (zoom_/DefaultZoom);

	const double src_submerge = attacker.is_flying() ? 0 : map_.get_terrain_info(src_terrain).unit_submerge();
	const double dst_submerge = attacker.is_flying() ? 0 : map_.get_terrain_info(dst_terrain).unit_submerge();

	for(int i = begin_at; i < end_at; i += time_resolution*acceleration) {
		events::pump();

		//this is a while instead of an if, because there might be multiple
		//sounds playing simultaneously or close together
		while(!hide && sfx_it != sounds.end() && i >= sfx_it->time) {
			const std::string& sfx = hits ? sfx_it->on_hit : sfx_it->on_miss;
			if(sfx.empty() == false) {
				sound::play_sound(hits ? sfx_it->on_hit : sfx_it->on_miss);
			}

			++sfx_it;
		}

		if(!hide && hits && !played_hit_sound && i >= play_hit_sound_at) {
			sound::play_sound(hit_sound);
			played_hit_sound = true;
		}

		for(int j = 0; j != 6; ++j) {
			draw_tile(update_tiles[j].x,update_tiles[j].y);
		}

		Uint32 defender_colour = 0;
		double defender_alpha = 1.0;

		if(damage > 0 && i == 0) {
			char buf[50];
			sprintf(buf,"%d",damage);
			float_label(b,buf,255,0,0);
		}

		if(damage > 0 && i >= 0) {
			if(def->second.gets_hit(minimum<int>(drain_speed,damage))) {
				dead = true;
				damage = 0;
			} else {
				damage -= drain_speed;
			}

			if(flash_num == 0 || flash_num == 2) {
				defender_alpha = 0.0;
				defender_colour = rgb(200,0,0);
			}

			++flash_num;
		}

		draw_tile(b.x,b.y,NULL,defender_alpha,defender_colour);

		int xoffset = 0;
		const std::string* unit_image = attack.get_frame(i,&xoffset);
		if(!attacker.facing_left())
			xoffset *= -1;

		xoffset = int(double(xoffset)*(zoom_/DefaultZoom));

		if(unit_image == NULL)
			unit_image = &attacker.image();

		const scoped_sdl_surface image((unit_image == NULL) ? NULL : image::get_image(*unit_image));

		const double pos = double(i)/double(i < 0 ? begin_at : end_at);
		const int posx = int(pos*xsrc + (1.0-pos)*xdst) + xoffset;
		const int posy = int(pos*ysrc + (1.0-pos)*ydst);

		const int height_adjust = int(src_height_adjust*pos + dst_height_adjust*(1.0-pos));
		const double submerge = src_submerge*pos + dst_submerge*(1.0-pos);

		if(image != NULL && !hide)
			draw_unit(posx,posy-height_adjust,image,attacker.facing_left(),false,1.0,0,submerge);

		const int wait_time = ticks + time_resolution - SDL_GetTicks();
		if(wait_time > 0 && !hide)
			SDL_Delay(wait_time);

		ticks = SDL_GetTicks();

		update_display();
	}

	hiddenUnit_ = gamemap::location();
	def->second.set_defending(false);

	draw_tile(a.x,a.y);
	draw_tile(b.x,b.y);

	if(dead) {
		unit_die(b);
	}

	return dead;
}

void display::unit_die(const gamemap::location& loc, SDL_Surface* image)
{
	if(update_locked() || fogged(loc.x,loc.y)
	   || preferences::show_combat() == false)
		return;

	const unit_map::const_iterator u = units_.find(loc);
	assert(u != units_.end());

	const std::string& die_sound = u->second.type().die_sound();
	if(die_sound != "" && die_sound != "null") {
		sound::play_sound(die_sound);
	}

	const int frame_time = 30;
	int ticks = SDL_GetTicks();

	for(double alpha = 1.0; alpha > 0.0; alpha -= 0.05) {
		draw_tile(loc.x,loc.y,image,alpha);

		const int wait_time = ticks + frame_time - SDL_GetTicks();

		if(wait_time > 0 && !turbo())
			SDL_Delay(wait_time);

		ticks = SDL_GetTicks();

		update_display();
	}

	draw(true,true);
}

void display::move_unit_between(const gamemap::location& a,
                                const gamemap::location& b,
								const unit& u)
{
	if(update_locked() || team_valid()
	                   && teams_[currentTeam_].fogged(a.x,a.y)
	                   && teams_[currentTeam_].fogged(b.x,b.y))
		return;

	const bool face_left = u.facing_left();

	const double side_threshhold = 80.0;

	double xsrc = get_location_x(a);
	double ysrc = get_location_y(a);
	double xdst = get_location_x(b);
	double ydst = get_location_y(b);

	const gamemap::TERRAIN src_terrain = map_.get_terrain(a);
	const gamemap::TERRAIN dst_terrain = map_.get_terrain(b);

	const int src_height_adjust = u.is_flying() ? 0 : int(map_.get_terrain_info(src_terrain).unit_height_adjust() * (zoom_/DefaultZoom));
	const int dst_height_adjust = u.is_flying() ? 0 : int(map_.get_terrain_info(dst_terrain).unit_height_adjust() * (zoom_/DefaultZoom));

	const double src_submerge = u.is_flying() ? 0 : int(map_.get_terrain_info(src_terrain).unit_submerge());
	const double dst_submerge = u.is_flying() ? 0 : int(map_.get_terrain_info(dst_terrain).unit_submerge());

	const double nsteps = turbo() ? 3.0 : 10.0;
	const double xstep = (xdst - xsrc)/nsteps;
	const double ystep = (ydst - ysrc)/nsteps;

	const int time_between_frames = turbo() ? 2 : 10;
	int ticks = SDL_GetTicks();

	int skips = 0;

	for(double i = 0.0; i < nsteps; i += 1.0) {
		events::pump();

		const scoped_sdl_surface image(image::get_image(u.type().image()));
		if(image == NULL) {
			std::cerr << "failed to get image " << u.type().image() << "\n";
			return;
		}

		xsrc = get_location_x(a);
		ysrc = get_location_y(a);
		xdst = get_location_x(b);
		ydst = get_location_y(b);

		double xloc = xsrc + xstep*i;
		double yloc = ysrc + ystep*i;

		//we try to scroll the map if the unit is at the edge.
		//keep track of the old position, and if the map moves at all,
		//then recenter it on the unit
		double oldxpos = xpos_;
		double oldypos = ypos_;
		if(xloc < side_threshhold) {
			scroll(xloc - side_threshhold,0.0);
		}

		if(yloc < side_threshhold) {
			scroll(0.0,yloc - side_threshhold);
		}

		if(xloc + double(image->w) > this->mapx() - side_threshhold) {
			scroll(((xloc + double(image->w)) -
			        (this->mapx() - side_threshhold)),0.0);
		}

		if(yloc + double(image->h) > this->y() - side_threshhold) {
			scroll(0.0,((yloc + double(image->h)) -
			           (this->y() - side_threshhold)));
		}

		if(oldxpos != xpos_ || oldypos != ypos_) {
			scroll_to_tile(b.x,b.y,WARP);
		}

		xsrc = get_location_x(a);
		ysrc = get_location_y(a);
		xdst = get_location_x(b);
		ydst = get_location_y(b);

		xloc = xsrc + xstep*i;
		yloc = ysrc + ystep*i;

		//invalidate the source tile and all adjacent tiles,
		//since the unit can partially overlap adjacent tiles
		gamemap::location adjacent[6];
		get_adjacent_tiles(a,adjacent);
		draw_tile(a.x,a.y);
		for(int tile = 0; tile != 6; ++tile) {
			draw_tile(adjacent[tile].x,adjacent[tile].y);
		}

		const int height_adjust = src_height_adjust + (dst_height_adjust-src_height_adjust)*(i/nsteps);
		const double submerge = src_submerge + (dst_submerge-src_submerge)*(i/nsteps);

		draw(false);
		draw_unit((int)xloc,(int)yloc - height_adjust,image,face_left,false,1.0,0,submerge);

		const int new_ticks = SDL_GetTicks();
		const int wait_time = time_between_frames - (new_ticks - ticks);
		if(wait_time > 0) {
			SDL_Delay(wait_time);
		}

		ticks = SDL_GetTicks();

		if(wait_time >= 0 || skips == 4 || (i+1.0) >= nsteps) {
			skips = 0;
			update_display();
		} else {
			++skips;
		}
	}
}

void display::draw_unit(int x, int y, SDL_Surface* image,
                        bool reverse, bool upside_down,
                        double alpha, Uint32 blendto, double submerged,
						SDL_Surface* ellipse_back, SDL_Surface* ellipse_front)
{
	if(ellipse_back != NULL) {
		draw_unit(x,y,ellipse_back,false,false,blendto == 0 ? alpha : 1.0,0,submerged);
	}

	sdl_add_ref(image);
	scoped_sdl_surface surf(image);

	if(upside_down) {
		surf.assign(flop_surface(surf));
	}

	if(!reverse) {
		surf.assign(flip_surface(surf));
	}

	if(alpha > 1.0) {
		surf.assign(brighten_image(surf,alpha));
	} else if(alpha != 1.0 && blendto != 0) {
		surf.assign(blend_surface(surf,1.0-alpha,blendto));
	} else if(alpha != 1.0) {
		surf.assign(adjust_surface_alpha(surf,alpha));
	}

	if(surf == NULL) {
		std::cerr << "surface lost...\n";
		return;
	}

	const int submerge_height = minimum<int>(surf->h,maximum<int>(0,surf->h*(1.0-submerged)));

	SDL_Rect clip_rect = map_area();
	SDL_Rect srcrect = {0,0,surf->w,submerge_height};
	blit_surface(x,y,surf,&srcrect,&clip_rect);

	if(submerge_height != surf->h) {
		surf.assign(adjust_surface_alpha(surf,0.2));
		
		srcrect.y = submerge_height;
		srcrect.h = surf->h-submerge_height;
		y += submerge_height;

		blit_surface(x,y,surf,&srcrect,&clip_rect);
	}

	if(ellipse_front != NULL) {
		draw_unit(x,y,ellipse_front,false,false,blendto == 0 ? alpha : 1.0,0,submerged);
	}
}

struct is_energy_colour {
	bool operator()(Uint32 colour) const { return (colour&0xFF000000) < 0x99000000 && (colour&0x00FF0000) > 0x00990000; }
};

const SDL_Rect& display::calculate_energy_bar()
{
	if(energy_bar_rect_.x != -1) {
		return energy_bar_rect_;
	}

	int first_row = -1, last_row = -1, first_col = -1, last_col = -1;

	scoped_sdl_surface image(image::get_image(game_config::unmoved_energy_image,image::SCALED));
	image.assign(make_neutral_surface(image));

	surface_lock image_lock(image);
	const Uint32* const begin = image_lock.pixels();

	for(int y = 0; y != image->h; ++y) {
		const Uint32* const i1 = begin + image->w*y;
		const Uint32* const i2 = i1 + image->w;
		const Uint32* const itor = std::find_if(i1,i2,is_energy_colour());
		const int count = std::count_if(itor,i2,is_energy_colour());

		if(itor != i2) {
			if(first_row == -1)
				first_row = y;

			first_col = itor - i1;
			last_col = first_col + count;
			last_row = y;
		}
	}

	const SDL_Rect res = {first_col,first_row,last_col-first_col,last_row+1-first_row};

	energy_bar_rect_ = res;
	return energy_bar_rect_;
}

void display::invalidate(const gamemap::location& loc)
{
	if(!invalidateAll_) {
		invalidated_.insert(loc);
	}
}

void display::invalidate_all()
{
	invalidateAll_ = true;
	invalidated_.clear();
	update_rect(map_area());
}

void display::invalidate_unit()
{
	invalidateUnit_ = true;
}

void display::recalculate_minimap()
{
	if(minimap_ != NULL) {
		SDL_FreeSurface(minimap_);
		minimap_ = NULL;
	}

	redraw_minimap();
}

void display::redraw_minimap()
{
	redrawMinimap_ = true;
}

void display::invalidate_game_status()
{
	invalidateGameStatus_ = true;
}

void display::add_overlay(const gamemap::location& loc, const std::string& img)
{
	overlays_.insert(std::pair<gamemap::location,std::string>(loc,img));
}

void display::remove_overlay(const gamemap::location& loc)
{
	overlays_.erase(loc);
}

void display::write_overlays(config& cfg) const
{
	for(std::multimap<gamemap::location,std::string>::const_iterator i = overlays_.begin();
	    i != overlays_.end(); ++i) {
		config& item = cfg.add_child("item");
		i->first.write(item);
		item["image"] = i->second;
	}
}

void display::set_team(size_t team)
{
	assert(team < teams_.size());
	currentTeam_ = team;

	labels().recalculate_shroud();
}

void display::set_playing_team(size_t team)
{
	assert(team < teams_.size());
	activeTeam_ = team;
	invalidate_game_status();
}

void display::set_advancing_unit(const gamemap::location& loc, double amount)
{
	advancingUnit_ = loc;
	advancingAmount_ = amount;
	draw_tile(loc.x,loc.y);
}

void display::lock_updates(bool value)
{
	if(value == true)
		++updatesLocked_;
	else
		--updatesLocked_;
}

bool display::update_locked() const
{
	return updatesLocked_ > 0;
}

bool display::turbo() const
{
	bool res = turbo_;
	if(keys_[SDLK_LSHIFT] || keys_[SDLK_RSHIFT])
		res = !res;

	return res;
}

void display::set_turbo(bool turbo)
{
	turbo_ = turbo;
}

void display::set_grid(bool grid)
{
	grid_ = grid;
}

void display::debug_highlight(const gamemap::location& loc, double amount)
{
	assert(game_config::debug);
	debugHighlights_[loc] += amount;
}

void display::clear_debug_highlights()
{
	debugHighlights_.clear();
}

bool display::shrouded(int x, int y) const
{
	if(team_valid())
		return teams_[currentTeam_].shrouded(x,y);
	else
		return false;
}

bool display::fogged(int x, int y) const
{
	if(team_valid())
		return teams_[currentTeam_].fogged(x,y);
	else
		return false;
}

bool display::team_valid() const
{
	return currentTeam_ < teams_.size();
}

size_t display::viewing_team() const
{
	return currentTeam_;
}

size_t display::playing_team() const
{
	return activeTeam_;
}

const theme& display::get_theme() const
{
	return theme_;
}

const theme::menu* display::menu_pressed(int mousex, int mousey, bool button_pressed)
{

	for(std::vector<gui::button>::iterator i = buttons_.begin(); i != buttons_.end(); ++i) {
		if(i->process(mousex,mousey,button_pressed)) {
			const size_t index = i - buttons_.begin();
			assert(index < theme_.menus().size());
			return &theme_.menus()[index];
		}
	}

	return NULL;
}

void display::create_buttons()
{
	buttons_.clear();

	const std::vector<theme::menu>& buttons = theme_.menus();
	for(std::vector<theme::menu>::const_iterator i = buttons.begin(); i != buttons.end(); ++i) {
		gui::button b(*this,i->title(),gui::button::TYPE_PRESS,i->image());
		const SDL_Rect& loc = i->location(screen_area());
		b.set_xy(loc.x,loc.y);
		buttons_.push_back(b);
	}
}

void display::add_observer(const std::string& name)
{
	observers_.insert(name);
}

void display::remove_observer(const std::string& name)
{
	observers_.erase(name);
}

namespace {
	const int chat_message_spacing = 20;
	const int max_chat_messages = 4;
	const int chat_message_x = 10;
	const int chat_message_y = 10;
	const SDL_Color chat_message_colour = {200,200,200,200};
}

void display::add_chat_message(const std::string& speaker, const std::string& msg, display::MESSAGE_TYPE type)
{
	std::stringstream str;
	if(type == MESSAGE_PUBLIC) {
		str << "<" << speaker << "> " << msg;
	} else {
		str << "*" << speaker << "* " << msg;
	}

	std::cerr << "chat message '" << str.str() << "'\n";
	const SDL_Rect rect = map_area();
	const int handle = font::add_floating_label(str.str(),12,chat_message_colour,
		rect.x+chat_message_x,rect.y+chat_message_y+chat_message_spacing*chat_messages_.size(),0,0,-1,rect,font::LEFT_ALIGN);
	std::cerr << "Added label..\n";
	chat_messages_.push_back(chat_message(handle));

	prune_chat_messages();
	std::cerr << "pruned messages...\n";
}

void display::prune_chat_messages(bool remove_all)
{
	const int message_ttl = remove_all ? 0 : 1200000;
	if(chat_messages_.empty() == false && (chat_messages_.front().created_at+message_ttl < SDL_GetTicks() || chat_messages_.size() > max_chat_messages)) {
		font::remove_floating_label(chat_messages_.front().handle);
		chat_messages_.erase(chat_messages_.begin());

		for(std::vector<chat_message>::const_iterator i = chat_messages_.begin(); i != chat_messages_.end(); ++i) {
			font::move_floating_label(i->handle,0,-chat_message_spacing);
		}

		prune_chat_messages(remove_all);
	}
}