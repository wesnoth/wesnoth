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

	display::Pixel alpha_blend_pixels(display::Pixel p1, display::Pixel p2,
	                         const SDL_PixelFormat* fmt, double alpha)
	{
		const int r1 = ((p1&fmt->Rmask) >> fmt->Rshift) << fmt->Rloss;
		const int g1 = ((p1&fmt->Gmask) >> fmt->Gshift) << fmt->Gloss;
		const int b1 = ((p1&fmt->Bmask) >> fmt->Bshift) << fmt->Bloss;

		const int r2 = ((p2&fmt->Rmask) >> fmt->Rshift) << fmt->Rloss;
		const int g2 = ((p2&fmt->Gmask) >> fmt->Gshift) << fmt->Gloss;
		const int b2 = ((p2&fmt->Bmask) >> fmt->Bshift) << fmt->Bloss;

		int r = int(r1*alpha);
		int g = int(g1*alpha);
		int b = int(b1*alpha);

		if(alpha < 1.0) {
			r += int(r2*(1.0-alpha));
			g += int(g2*(1.0-alpha));
			b += int(b2*(1.0-alpha));
		} else {
			if(r > r2)
				r = r2;

			if(g > g2)
				g = g2;

			if(b > b2)
				b = b2;
		}

		return ((r >> fmt->Rloss) << fmt->Rshift) |
		       ((g >> fmt->Gloss) << fmt->Gshift) |
		       ((b >> fmt->Bloss) << fmt->Bshift);
	}

	const size_t SideBarText_x = 13;
	const size_t SideBarUnit_y = 435;
	const size_t SideBarUnitProfile_y = 375;
	const size_t SideBarGameStatus_x = 16;
	const size_t SideBarGameStatus_y = 220;
	const size_t TimeOfDay_x = 13;
	const size_t TimeOfDay_y = 167;

	const SDL_Rect empty_rect = {0,0,0,0};
}

display::display(unit_map& units, CVideo& video, const gamemap& map,
				 const gamestatus& status, const std::vector<team>& t, const config& theme_cfg)
		             : screen_(video), xpos_(0.0), ypos_(0.0),
					   zoom_(DefaultZoom), map_(map), units_(units),
					   energy_bar_count_(-1,-1), minimap_(NULL),
					   pathsList_(NULL), status_(status),
                       teams_(t), lastDraw_(0), drawSkips_(0),
					   invalidateAll_(true), invalidateUnit_(true),
					   invalidateGameStatus_(true), panelsDrawn_(false),
					   currentTeam_(0), activeTeam_(0), updatesLocked_(0),
                       turbo_(false), grid_(false), sidebarScaling_(1.0),
					   theme_(theme_cfg,screen_area())
{
	create_buttons();

	std::fill(reportRects_,reportRects_+reports::NUM_REPORTS,empty_rect);

	new_turn();

	image::set_zoom(zoom_);

	gameStatusRect_.w = 0;
	unitDescriptionRect_.w = 0;
	unitProfileRect_.w = 0;

	//clear the screen contents
	SDL_Surface* const disp = screen_.getSurface();
	const int length = disp->w*disp->h;

	surface_lock lock(disp);
	short* const pixels = lock.pixels();
	std::fill(pixels,pixels+length,0);
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
}

void display::new_turn()
{
	adjust_colours(0,0,0);
}

void display::adjust_colours(int r, int g, int b)
{
	const time_of_day& tod = status_.get_time_of_day();
	image::set_colour_adjustment(tod.red+r,tod.green+g,tod.blue+b);
}

gamemap::location display::hide_unit(const gamemap::location& loc)
{
	const gamemap::location res = hiddenUnit_;
	hiddenUnit_ = loc;
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

	const double xtile = xpos_/(zoom_*0.75) +
			               static_cast<double>(xclick)/(zoom_*0.75) - 0.25;
	const double ytile = ypos_/zoom_ + static_cast<double>(yclick)/zoom_
	                      + (is_odd(int(xtile)) ? -0.5:0.0);

	return gamemap::location(static_cast<int>(xtile),static_cast<int>(ytile));
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

display::Pixel display::rgb(int r, int g, int b) const
{
	return SDL_MapRGB(const_cast<display*>(this)->video().getSurface()->format,
	                  r,g,b);
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
		invalidate_all();
	}
}

void display::zoom(double amount)
{
	energy_bar_count_ = std::pair<int,int>(-1,-1);

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
		xpos_ = orig_xpos;
		ypos_ = orig_ypos;
		zoom_ = orig_zoom;
		return;
	}

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
	const double min_zoom = min_zoom1 > min_zoom2 ? min_zoom1 : min_zoom2;
	const double max_zoom = 200.0;

	const int orig_zoom = int(zoom_);

	zoom_ = floor(zoom_);

	if(zoom_ < min_zoom) {
		zoom_ = min_zoom;
	}

	if(zoom_ > max_zoom) {
		zoom_ = max_zoom;
	}

	const double xend = zoom_*map_.x()*0.75 + zoom_*0.25;
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
		std::cerr << "invalidating reports...\n";
		std::fill(reports_,reports_+sizeof(reports_)/sizeof(*reports_),reports::report());
		invalidateGameStatus_ = true;
		panelsDrawn_ = true;
	}

	if(invalidateAll_ && !map_.empty()) {
		for(int x = -1; x <= map_.x(); ++x)
			for(int y = -1; y <= map_.y(); ++y)
				draw_tile(x,y);
		invalidateAll_ = false;
		const SDL_Rect area = minimap_area();
		draw_minimap(area.x,area.y,area.w,area.h);
	} else if(!map_.empty()) {
		for(std::set<gamemap::location>::const_iterator it =
		    invalidated_.begin(); it != invalidated_.end(); ++it) {
			draw_tile(it->x,it->y);
		}

		invalidated_.clear();
	}

	if(!map_.empty()) {
		draw_sidebar();
	}

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

void display::draw_report(reports::TYPE report_num)
{
	if(!team_valid())
		return;

	const theme::status_item* const item = theme_.get_status_item(reports::report_name(report_num));
	if(item != NULL) {

		const reports::report& report = reports::generate_report(report_num,map_,
				units_, teams_,
		      teams_[viewing_team()],
				currentTeam_+1,activeTeam_+1,
				selectedHex_,mouseoverHex_,status_);

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
			if(rect.w > 0 && rect.h > 0 && report.text.empty() == false) {
				surf.assign(get_surface_portion(screen_.getSurface(),rect));
				if(reportSurfaces_[report_num] == NULL) {
					std::cerr << "Could not backup background for report!\n";
				}
			}

			update_rect(rect);
		}

		tooltips::clear_tooltips(rect);

		SDL_Rect area = rect;

		if(report.text.empty() == false) {
			std::string str = item->prefix();

			int nchop;

			//if there are formatting directives on the front of the report,
			//move them to the front of the string
			for(nchop = 0; nchop != report.text.size() && font::is_format_char(report.text[nchop]); ++nchop) {
				str.insert(str.begin(),report.text[0]);
			}

			str += report.text.substr(nchop) + item->postfix();

			area = font::draw_text(this,rect,item->font_size(),font::NORMAL_COLOUR,str,rect.x,rect.y);
		}

		if(report.tooltip.empty() == false) {
			tooltips::add_tooltip(area,report.tooltip);
		}

		if(report.image.empty() == false) {

			scoped_sdl_surface img(image::get_image(report.image,image::UNSCALED));
			if(img == NULL) {
				std::cerr << "could not find image for report: '" << report.image << "'\n";
				return;
			}

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

		const std::string& lang_weapon
		         = string_table["weapon_name_" + at_it->name()];
		const std::string& lang_type
		         = string_table["weapon_type_" + at_it->type()];
		const std::string& lang_special
		         = string_table["weapon_special_" + at_it->special()];
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
	x = minimap_location.x;
	y = minimap_location.y;
	w = minimap_location.w;
	h = minimap_location.h;

	SDL_BlitSurface(surface,NULL,screen_.getSurface(),&minimap_location);

	const double xscaling = double(surface->w)/double(map_.x());
	const double yscaling = double(surface->h)/double(map_.y());

	const int xbox = static_cast<int>(xscaling*xpos_/(zoom_*0.75));
	const int ybox = static_cast<int>(yscaling*ypos_/zoom_);

	const int wbox = static_cast<int>(xscaling*map_area().w/(zoom_*0.75) - xscaling);
	const int hbox = static_cast<int>(yscaling*map_area().h/zoom_ - yscaling);

	const Pixel boxcolour = Pixel(SDL_MapRGB(surface->format,0xFF,0xFF,0xFF));
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
			std::cout << "terrain palette can't fit: " << x + image->w << " > " << this->x() << " or " << y+image->h << " > " << this->y() << "\n";
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

void display::draw_tile(int x, int y, SDL_Surface* unit_image_override,
                        double highlight_ratio, Pixel blend_with)

{
	if(updatesLocked_)
		return;

	const gamemap::location loc(x,y);
	int xpos = int(get_location_x(loc));
	int ypos = int(get_location_y(loc));

	if(xpos >= int(map_area().x + map_area().w) || ypos >= int(map_area().y + map_area().h))
		return;

	int xend = xpos + static_cast<int>(zoom_);
	int yend = int(get_location_y(gamemap::location(x,y+1)));

	if(xend < int(map_area().x) || yend < int(map_area().y))
		return;

	const gamemap::location ne_loc(loc.get_direction(gamemap::location::NORTH_EAST));
	const gamemap::location se_loc(loc.get_direction(gamemap::location::SOUTH_EAST));

	const int ne_xpos = int(get_location_x(ne_loc));
	const int ne_ypos = int(get_location_y(ne_loc));
	const int se_xpos = int(get_location_x(se_loc));
	const int se_ypos = int(get_location_y(se_loc));

	const bool is_shrouded = shrouded(x,y);
	gamemap::TERRAIN terrain = gamemap::VOID_TERRAIN;

	if(!is_shrouded) {
		terrain = map_.get_terrain(loc);
	}

	image::TYPE image_type = image::SCALED;

	if(fogged(x,y)) {
		image_type = image::FOGGED;
	}

	//find if this tile should be greyed
	if(pathsList_ != NULL && pathsList_->routes.find(gamemap::location(x,y)) ==
					         pathsList_->routes.end()) {
		image_type = image::GREYED;
	}

	if(loc == mouseoverHex_ && map_.on_board(mouseoverHex_) ||
	   loc == selectedHex_ && units_.count(gamemap::location(x,y)) == 1) {
		image_type = image::BRIGHTENED;
	}

	const scoped_sdl_surface surface(getTerrain(terrain,image_type,x,y));

	if(surface == NULL) {
		std::cerr << "Could not get terrain surface\n";
		return;
	}

	std::vector<SDL_Surface*> overlaps;

	scoped_sdl_surface flag(NULL);

	if(!is_shrouded) {
		flag.assign(getFlag(terrain,x,y));
		if(flag != NULL)
			overlaps.push_back(flag);

		const std::vector<SDL_Surface*>& adj = getAdjacentTerrain(x,y,image_type);

		overlaps.insert(overlaps.end(),adj.begin(),adj.end());

		typedef std::multimap<gamemap::location,std::string>::const_iterator
		        Itor;

		for(std::pair<Itor,Itor> overlays =
		    overlays_.equal_range(gamemap::location(x,y));
			overlays.first != overlays.second; ++overlays.first) {

			//event though the scoped surface will fall out-of-scope and call
			//SDL_FreeSurface on the underlying surface, the surface will remain
			//valid so long as the image cache isn't invalidated, which should not
			//happen inside this function
			const scoped_sdl_surface overlay_surface(image::get_image(overlays.first->second));

			if(overlay_surface != NULL) {
				overlaps.push_back(overlay_surface);
			}
		}
	}

	int ysrc = 0, xsrc = 0;

	if(xpos < int(map_area().x)) {
		xsrc += map_area().x - xpos;
		xpos = map_area().x;
	}

	if(ypos < int(map_area().y)) {
		ysrc += map_area().y - ypos;
		ypos = map_area().y;
	}

	if(xend > map_area().x + map_area().w) {
		xend = map_area().x + map_area().w;
	}

	if(yend > map_area().y + map_area().h) {
		yend = map_area().y + map_area().h;
	}

	if(xend <= xpos || yend <= ypos)
		return;

	update_rect(xpos,ypos,xend-xpos,yend-ypos);

	double unit_energy = 0.0;

	const short energy_loss_colour = 0;
	short energy_colour = 0;

	const int max_energy = 80;
	double energy_size = 1.0;
	bool face_left = true;

	if(unit_image_override != NULL)
		sdl_add_ref(unit_image_override);

	scoped_sdl_surface unit_image(unit_image_override);
	scoped_sdl_surface energy_image(NULL);

	//see if there is a unit on this tile
	const unit_map::const_iterator it = units_.find(gamemap::location(x,y));
	if(it != units_.end()) {
		if(unit_image == NULL)
			unit_image.assign(image::get_image(it->second.image()));

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

		energy_image.assign(image::get_image(*energy_file));
		if(energy_image.get() == NULL) {
			std::cerr << "failed to get energy image: '" << *energy_file << "'\n";
			return;
		}

		unit_energy = double(it->second.hitpoints()) /
		              double(it->second.max_hitpoints());

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
			blend_with = short(0xFFFF);
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

			energy_colour = ::SDL_MapRGB(energy_image->format,er,eg,eb);
		}

		if(it->second.max_hitpoints() < max_energy) {
			energy_size = double(it->second.max_hitpoints())/double(max_energy);
		}

		face_left = it->second.facing_left();
	}

	const std::pair<int,int>& energy_bar_loc = calculate_energy_bar();
	double total_energy = double(energy_bar_loc.second - energy_bar_loc.first);

	const int skip_energy_rows = int(total_energy*(1.0-energy_size));
	total_energy -= double(skip_energy_rows);

	const int show_energy_after = energy_bar_loc.first +
	                              int((1.0-unit_energy)*total_energy);

	const bool draw_hit = hitUnit_ == gamemap::location(x,y);
	const short hit_colour = short(0xF000);

	if(deadUnit_ == gamemap::location(x,y)) {
		highlight_ratio = deadAmount_;
	}

	SDL_Surface* const dst = screen_.getSurface();

	const Pixel grid_colour = SDL_MapRGB(dst->format,0,0,0);

	int j;
	for(j = ypos; j != yend; ++j) {

		//store the number of pixel-rows into the hex we are in yloc
		const int yloc = ysrc+j-ypos;
		const int xoffset = abs(yloc - static_cast<int>(zoom_/2.0))/2;

		//store the number of pixels-rows we are into the north-east hex
		const int ne_yloc = j-ne_ypos;
		const int ne_xoffset = abs(ne_yloc - static_cast<int>(zoom_/2.0))/2;

		//store the number of pixels-rows we are into the south-east hex
		const int se_yloc = j-se_ypos;
		const int se_xoffset = abs(se_yloc - static_cast<int>(zoom_/2.0))/2;

		int xdst = xpos;
		if(xoffset > xsrc) {
			xdst += xoffset - xsrc;
			if(xend < xdst)
				continue;
		}

		const int maxlen = static_cast<int>(zoom_) - xoffset*2;
		int len = minimum(xend - xdst,maxlen);

		const int neoffset = ne_xpos+ne_xoffset;
		const int seoffset = se_xpos+se_xoffset;
		const int minoffset = minimum<int>(neoffset,seoffset);

		//FIXME: make it work with ne_ypos being <= 0
		if(ne_ypos > 0 && xdst + len >= neoffset) {
			len = neoffset - xdst;
			if(len < 0)
				len = 0;
		} else if(ne_ypos > 0 && xdst + len >= seoffset) {
			len = seoffset - xdst;
			if(len < 0)
				len = 0;
		}

		const int srcy = minimum<int>(yloc,surface->h-1);
		assert(srcy >= 0);

		const int diff = maximum<int>(0,srcy*surface->w + maximum<int>(xoffset,xsrc));
		len = minimum<int>(len,surface->w*surface->h - diff);
		if(len <= 0) {
			continue;
		}

		SDL_Rect srcrect = { maximum<int>(xoffset,xsrc), srcy, len, 1 };
		SDL_Rect dstrect = { xdst, j, 0, 0 };

		SDL_BlitSurface(surface,&srcrect,dst,&dstrect);

		int extra = 0;

		SDL_Rect end_srcrect = { srcrect.x + srcrect.w - 1, srcrect.y, 1, 1 };

		//if the line didn't make it to the next hex, then fill in with the
		//last pixel up to the next hex
		if(ne_ypos > 0 && xdst + len < minoffset && len > 0) {
			extra = minimum(minoffset-(xdst + len),map_area().x+map_area().w-(xdst+len));
			SDL_Rect rect = { dstrect.x + len, dstrect.y, 1, 1 };
			for(int n = 0; n != extra; ++n, ++rect.x) {
				SDL_BlitSurface(surface,&end_srcrect,dst,&rect);
			}
		}

		//copy any overlapping tiles on
		for(std::vector<SDL_Surface*>::const_iterator ov = overlaps.begin();
		    ov != overlaps.end(); ++ov) {
			SDL_BlitSurface(*ov,&srcrect,dst,&dstrect);

			for(int i = 0; i != extra; ++i) {
				SDL_Rect rect = { dstrect.x + len + i, dstrect.y, 1, 1 };
				SDL_BlitSurface(*ov,&end_srcrect,dst,&rect);
			}
		}

		if(grid_ && srcrect.w >= 1) {
			SDL_Rect rect = dstrect;
			if(j == ypos || j == yend-1) {
				SDL_FillRect(dst,&rect,grid_colour);
			} else {
				rect.w = 1;
				rect.h = 1;
				SDL_FillRect(dst,&rect,grid_colour);
				rect.x += srcrect.w+extra-1;
				SDL_FillRect(dst,&rect,grid_colour);
			}
		}
	}

	if(game_config::debug && debugHighlights_.count(gamemap::location(x,y))) {
		const scoped_sdl_surface cross(image::get_image(game_config::cross_image));
		if(cross != NULL)
			draw_unit(xpos-xsrc,ypos-ysrc,cross,face_left,false,
			          debugHighlights_[loc],0);
	}

	if(!is_shrouded) {
		draw_footstep(loc,xpos-xsrc,ypos-ysrc);
	}

	if(unit_image == NULL || energy_image == NULL || fogged(x,y) ||
			(teams_[currentTeam_].is_enemy(it->second.side()) && 
			it->second.invisible(map_.underlying_terrain(map_[x][y]), 
					status_.get_time_of_day().lawful_bonus,loc,
					units_,teams_))) {
		return;
	}

	if(loc != hiddenUnit_) {
		if(draw_hit) {
			blend_with = hit_colour;
			highlight_ratio = 0.7;
		} else if(loc == advancingUnit_ && it != units_.end()) {
			//the unit is advancing - set the advancing colour to white if it's a
			//non-chaotic unit, otherwise black
			blend_with = it->second.type().alignment() == unit_type::CHAOTIC ?
			                                        0x0001 : 0xFFFF;
			highlight_ratio = advancingAmount_;
		} else if(it->second.poisoned()) {
			//the unit is poisoned - draw with a green hue
			blend_with = SDL_MapRGB(dst->format,0,255,0);
			highlight_ratio = 0.75;
		}

		const int height_adjust = it->second.is_flying() ? 0 : int(map_.get_terrain_info(terrain).unit_height_adjust()*(zoom_/DefaultZoom));
		const double submerge = it->second.is_flying() ? 0.0 : map_.get_terrain_info(terrain).unit_submerge();

		draw_unit(xpos-xsrc,ypos-ysrc - height_adjust,unit_image,face_left,false,
		          highlight_ratio,blend_with,submerge);

		//the circle around the base of the unit
		if(preferences::show_side_colours() && !fogged(x,y) && it != units_.end()) {
			const SDL_Color& col = font::get_side_colour(it->second.side());
			const short colour = SDL_MapRGB(dst->format,col.r,col.g,col.b);
			SDL_Rect clip = {xpos,ypos,xend-xpos,yend-ypos};

			draw_unit_ellipse(dst,colour,clip,xpos-xsrc,ypos-ysrc-height_adjust,unit_image,!face_left);
		}
	}

	const bool energy_uses_alpha = highlight_ratio < 1.0 && blend_with == 0;

	surface_lock dstlock(dst);
	surface_lock energy_lock(energy_image);

	for(j = ypos; j != yend; ++j) {
		const int yloc = ysrc+j-ypos;
		const int xoffset = abs(yloc - static_cast<int>(zoom_/2.0))/2;

		int xdst = xpos;
		if(xoffset > xsrc) {
			xdst += xoffset - xsrc;
			if(xend < xdst)
				continue;
		}

		const int maxlen = static_cast<int>(zoom_) - xoffset*2;
		int len = ((xend - xdst) > maxlen) ? maxlen : xend - xdst;

		short* startdst = dstlock.pixels() + j*dst->w + xdst;

		const short new_energy = yloc >= show_energy_after ?
		                             energy_colour : energy_loss_colour;

		const int skip = yloc >= energy_bar_loc.first ? skip_energy_rows:0;

		short* startenergy = NULL;

		const int energy_w = energy_image->w + is_odd(energy_image->w);
		if(yloc + skip < energy_image->h) {
			startenergy = energy_lock.pixels() + (yloc+skip)*energy_w +
			              maximum<int>(xoffset,xsrc);

			for(int i = 0; i != len; ++i) {
				Uint8 r, g, b;
				SDL_GetRGB(*startenergy,energy_image->format,&r,&g,&b);
				if(startenergy != NULL && *startenergy != 0) {
					if(!energy_uses_alpha) {
						if(r > 230 && g > 230 && b > 230) {
							*startdst = new_energy;
						} else {
							*startdst = *startenergy;
						}
					} else {
						Pixel p = *startenergy;
						if(r > 230 && g > 230 && b > 230) {
							p = new_energy;
						}
						*startdst = alpha_blend_pixels(p,*startdst,
						                      dst->format,highlight_ratio);
					}
				}

				++startdst;

				if(startenergy != NULL)
					++startenergy;
			}
		}
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
		const std::string str(1,'1' + route_.move_left);
		const SDL_Rect& text_area = font::draw_text(NULL,rect,18,text_colour,str,0,0);
		const int x = xloc + int(zoom_/2.0) - text_area.w/2;
		const int y = yloc + int(zoom_/2.0) - text_area.h/2;
		font::draw_text(this,rect,18,text_colour,str,x,y);
	}
}

namespace {
const std::string& get_direction(int n)
{
	static std::map<int,std::string> dirs;
	if(dirs.empty()) {
		dirs[0] = "-n";
		dirs[1] = "-ne";
		dirs[2] = "-se";
		dirs[3] = "-s";
		dirs[4] = "-sw";
		dirs[5] = "-nw";
	}

	return dirs[n];
}

}

std::vector<SDL_Surface*> display::getAdjacentTerrain(int x, int y,
													  image::TYPE image_type)
{
	std::vector<SDL_Surface*> res;
	gamemap::location loc(x,y);

	const gamemap::TERRAIN current_terrain = map_.get_terrain(loc);

	gamemap::location adjacent[6];
	get_adjacent_tiles(loc,adjacent);
	int tiles[6];
	for(int i = 0; i != 6; ++i) {
		tiles[i] = map_.get_terrain(adjacent[i]);
	}

	const std::vector<gamemap::TERRAIN>& precedence = map_.get_terrain_precedence();
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
				SDL_Surface* surface = NULL;
				std::ostringstream stream;
				for(int n = 0; tiles[i] == *terrain && n!=6; i = (i+1)%6, ++n) {
					stream << get_direction(i);
					const scoped_sdl_surface new_surface(getTerrain(
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

SDL_Surface* display::getTerrain(gamemap::TERRAIN terrain,image::TYPE image_type,
                                 int x, int y, const std::string& direction)
{
	std::string image = "terrain/" + (direction.empty() ?
	                           map_.get_terrain_info(terrain).image(x,y) :
	                           map_.get_terrain_info(terrain).adjacent_image());

	//see if there is a time-of-day specific version of this image
	if(direction == "") {
		const time_of_day& tod = status_.get_time_of_day();
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

	return im;
}

SDL_Surface* display::getFlag(gamemap::TERRAIN terrain, int x, int y)
{
	const bool village = (map_.underlying_terrain(terrain) == gamemap::TOWER);
	if(!village)
		return NULL;

	const gamemap::location loc(x,y);

	for(size_t i = 0; i != teams_.size(); ++i) {
		if(teams_[i].owns_tower(loc) && (!fogged(x,y) || i == currentTeam_)) {
			char buf[50];
			sprintf(buf,"terrain/flag-team%d.png",i+1);
			return image::get_image(buf);
		}
	}

	return NULL;
}

void display::blit_surface(int x, int y, SDL_Surface* surface)
{
	SDL_Surface* const target = video().getSurface();

	const int srcx = x < 0 ? -x : 0;
	const int srcw = x + surface->w > target->w ? target->w - x :
	                                              surface->w - srcx;
	const int srcy = y < 0 ? -y : 0;
	const int srch = y + surface->h > target->h ? target->h - x :
	                                              surface->h - srcy;

	if(srcw <= 0 || srch <= 0 || srcx >= surface->w || srcy >= surface->h)
		return;

	SDL_Rect src_rect = {srcx, srcy, srcw, srch};
	SDL_Rect dst_rect = {x, y, srcw, srch};

	SDL_BlitSurface(surface,&src_rect,target,&dst_rect);
}

SDL_Surface* display::getMinimap(int w, int h)
{
	if(minimap_ == NULL)
		minimap_ = image::getMinimap(w,h,map_,
				status_.get_time_of_day().lawful_bonus,
				team_valid() ? &teams_[currentTeam_] : NULL, &units_, &teams_);

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

double display::get_location_x(const gamemap::location& loc) const
{
	return map_area().x + static_cast<double>(loc.x)*zoom_*0.75 - xpos_;
}

double display::get_location_y(const gamemap::location& loc) const
{
	return map_area().y + static_cast<double>(loc.y)*zoom_ - ypos_ +
					         (is_odd(loc.x) ? zoom_/2.0 : 0.0);
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

		Pixel defensive_colour = 0;
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
				defensive_colour = short(0xF000);
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

		int defender_colour = 0;
		double defender_alpha = 1.0;

		if(damage > 0 && i >= 0) {
			if(def->second.gets_hit(minimum<int>(drain_speed,damage))) {
				dead = true;
				damage = 0;
			} else {
				damage -= drain_speed;
			}

			if(flash_num == 0 || flash_num == 2) {
				defender_alpha = 0.0;
				defender_colour = 0xF000;
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

	if(dead) {
		unit_die(b);
	}

	return dead;
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
                        double alpha, Pixel blendto, double submerged)
{
	//the alpha value to use for submerged units
	static const double unit_submerged_alpha = 0.2;

	if(updatesLocked_) {
		return;
	}

	const int w = map_area().x + map_area().w;
	const int h = map_area().y + map_area().h-1;
	if(x > w || y > h)
		return;

	const int image_w = image->w + is_odd(image->w);

	SDL_Surface* const screen = screen_.getSurface();

	surface_lock srclock(image);
	const Pixel* src = srclock.pixels();

	const int height = image->h;
	const int submerge_height = y + image->h*(1.0 - submerged);

	const int endy = (y + height) < h ? (y + height) : h;
	const int endx = (x + image->w) < w ? (x + image->w) : w;
	if(endx < x)
		return;

	const int len = endx - x;

	if(y < map_area().y) {
		src += image_w*(map_area().y - y);
		y = map_area().y;
		if(y >= endy)
			return;
	}

	int xoffset = 0;
	if(x < map_area().x) {
		xoffset = map_area().x - x;
		x = map_area().x;
		if(x >= endx)
			return;
	}

	const SDL_PixelFormat* const fmt = screen->format;

	static const Pixel semi_trans = ((0x19 >> fmt->Rloss) << fmt->Rshift) |
	                                ((0x19 >> fmt->Gloss) << fmt->Gshift) |
	                                ((0x11 >> fmt->Bloss) << fmt->Bshift);

	if(upside_down)
		src += image_w * (endy - y - 1);

	const int src_increment = image_w * (upside_down ? -1 : 1);

	surface_lock screen_lock(screen);

	const Pixel ShroudColour = 0;

	for(; y != endy; ++y, src += src_increment) {
		Pixel* dst = screen_lock.pixels() + y*screen->w + x;

		if(y == submerge_height && alpha > unit_submerged_alpha) {
			alpha = unit_submerged_alpha;
			blendto = 0;
		}

		if(alpha == 1.0) {
			if(reverse) {
				for(int i = xoffset; i != len; ++i) {
					if(dst[i-xoffset] == ShroudColour)
						continue;

					if(src[i] == semi_trans)
						dst[i-xoffset] = alpha_blend_pixels(
						                     0,dst[i-xoffset],fmt,0.5);
					else if(src[i] != 0)
						dst[i-xoffset] = src[i];
				}
			} else {
				for(int i = image->w-1-xoffset; i != image->w-len-1; --i,++dst){
					if(*dst == ShroudColour)
						continue;

					if(src[i] == semi_trans)
						*dst = alpha_blend_pixels(0,*dst,fmt,0.5);
					else if(src[i] != 0)
						*dst = src[i];
				}
			}
		} else {
			if(reverse) {
				for(int i = xoffset; i != len; ++i) {
					if(dst[i-xoffset] == ShroudColour)
						continue;

					const Pixel blend = blendto ? blendto : dst[i-xoffset];

					if(src[i] != 0)
						dst[i-xoffset]
						      = alpha_blend_pixels(src[i],blend,fmt,alpha);
				}
			} else {
				for(int i = image->w-1-xoffset; i != image->w-len-1; --i,++dst){
					if(*dst == ShroudColour)
						continue;

					const Pixel blend = blendto ? blendto : *dst;
					if(src[i] != 0)
						*dst = alpha_blend_pixels(src[i],blend,fmt,alpha);
				}
			}
		}
	}
}

const std::pair<int,int>& display::calculate_energy_bar()
{
	if(energy_bar_count_.first != -1) {
		return energy_bar_count_;
	}

	int first_row = -1;
	int last_row = -1;

	const scoped_sdl_surface image(image::get_image(game_config::unmoved_energy_image));

	surface_lock image_lock(image);
	const short* const begin = image_lock.pixels();

	const Pixel colour = Pixel(SDL_MapRGB(image->format,0xFF,0xFF,0xFF));

	for(int y = 0; y != image->h; ++y) {
		const short* const i1 = begin + image->w*y;
		const short* const i2 = i1 + image->w;
		if(std::find(i1,i2,colour) != i2) {
			if(first_row == -1)
				first_row = y;

			last_row = y;
		}
	}

	energy_bar_count_ = std::pair<int,int>(first_row,last_row+1);
	return energy_bar_count_;
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
