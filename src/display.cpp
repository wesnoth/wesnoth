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
	const size_t Minimap_x = 13;
	const size_t Minimap_y = 11;
	const size_t Minimap_w = 119;
	const size_t Minimap_h = 146;
	const size_t TimeOfDay_x = 13;
	const size_t TimeOfDay_y = 167;

	const std::string RightSideTop = "misc/rightside.png";
	const std::string RightSideBot = "misc/rightside-bottom.png";
}

display::display(unit_map& units, CVideo& video, const gamemap& map,
				 const gamestatus& status, const std::vector<team>& t)
		             : screen_(video), xpos_(0.0), ypos_(0.0),
					   zoom_(DefaultZoom), map_(map), units_(units),
					   energy_bar_count_(-1,-1), minimap_(NULL),
					   pathsList_(NULL), status_(status),
                       teams_(t), lastDraw_(0), drawSkips_(0),
					   invalidateAll_(true), invalidateUnit_(true),
					   invalidateGameStatus_(true), sideBarBgDrawn_(false),
					   currentTeam_(0), activeTeam_(0), updatesLocked_(0),
                       turbo_(false), grid_(false), sidebarScaling_(1.0)
{
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
	if(xclick > mapx())
		return gamemap::location();

	const double xtile = xpos_/(zoom_*0.75) +
			               static_cast<double>(xclick)/(zoom_*0.75) - 0.25;
	const double ytile = ypos_/zoom_ + static_cast<double>(yclick)/zoom_
	                      + (is_odd(int(xtile)) ? -0.5:0.0);

	return gamemap::location(static_cast<int>(xtile),static_cast<int>(ytile));
}

gamemap::location display::minimap_location_on(int x, int y)
{
	const SDL_Rect rect = {mapx()+Minimap_x,Minimap_y,Minimap_w,Minimap_h};

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

void display::scroll_to_tile(int x, int y, SCROLL_TYPE scroll_type)
{
	if(update_locked() || fogged(x,y))
		return;

	if(map_.on_board(gamemap::location(x,y)) == false)
		return;

	const double xpos = static_cast<double>(x)*zoom_*0.75 - xpos_;
	const double ypos = static_cast<double>(y)*zoom_ - ypos_ +
					                    ((x % 2) == 1 ? zoom_/2.0 : 0.0);

	const double speed = preferences::scroll_speed()*2.0;

	const double desiredxpos = this->mapx()/2.0 - zoom_/2.0;
	const double desiredypos = this->   y()/2.0 - zoom_/2.0;

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
                              SCROLL_TYPE scroll_type)
{
	const double xpos1 = static_cast<double>(x1)*zoom_*0.75 - xpos_;
	const double ypos1 = static_cast<double>(y1)*zoom_ - ypos_ +
					                    ((x1 % 2) == 1 ? zoom_/2.0 : 0.0);
	const double xpos2 = static_cast<double>(x2)*zoom_*0.75 - xpos_;
	const double ypos2 = static_cast<double>(y2)*zoom_ - ypos_ +
					                    ((x2 % 2) == 1 ? zoom_/2.0 : 0.0);

	const double diffx = fabs(xpos1 - xpos2);
	const double diffy = fabs(ypos1 - ypos2);

	if(diffx > mapx()/(zoom_*0.75) || diffy > y()/zoom_) {
		scroll_to_tile(x1,y1,scroll_type);
	} else {
		scroll_to_tile((x1+x2)/2,(y1+y2)/2,scroll_type);
	}
}

void display::bounds_check_position()
{
	const double min_zoom1 = static_cast<double>(mapx()/(map_.x()*0.75 + 0.25));
	const double min_zoom2 = static_cast<double>(y()/map_.y());
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

	if(xpos_ + static_cast<double>(mapx()) > xend)
		xpos_ -= xpos_ + static_cast<double>(mapx()) - xend;

	if(ypos_ + static_cast<double>(y()) > yend)
		ypos_ -= ypos_ + static_cast<double>(y()) - yend;

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

	tooltips::clear_tooltips();

	sideBarBgDrawn_ = false;
	invalidate_all();
	draw(true,true);
}

void display::draw(bool update,bool force)
{
	if(!sideBarBgDrawn_ && !teams_.empty()) {
		SDL_Surface* const screen = screen_.getSurface();
		SDL_Surface* image_top = image::get_image(RightSideTop,image::UNSCALED);


		SDL_Surface* image = image_top != NULL ?
		 image::get_image_dim(RightSideBot,image_top->w,screen->h-image_top->h)
		    : NULL;
		if(image_top != NULL && image != NULL && image_top->h < screen->h) {

			SDL_Rect dstrect;
			dstrect.x = mapx();
			dstrect.y = 0;
			dstrect.w = image_top->w;
			dstrect.h = image_top->h;

			if(dstrect.x + dstrect.w <= this->x() &&
			   dstrect.y + dstrect.h <= this->y()) {
				SDL_BlitSurface(image_top,NULL,screen,&dstrect);

				dstrect.y = image_top->h;
				dstrect.h = image->h;
				if(dstrect.y + dstrect.h <= this->y()) {
					SDL_BlitSurface(image,NULL,screen,&dstrect);
				}
			} else {
				std::cout << (dstrect.x+dstrect.w) << " > " << this->x() << " or " << (dstrect.y + dstrect.h) << " > " << this->y() << "\n";
			}
		}

		sideBarBgDrawn_ = true;

		update_rect(mapx(),0,this->x()-mapx(),this->y());
	}

	if(invalidateAll_ && !map_.empty()) {
		for(int x = -1; x <= map_.x(); ++x)
			for(int y = -1; y <= map_.y(); ++y)
				draw_tile(x,y);
		invalidateAll_ = false;
		draw_minimap(mapx()+Minimap_x,Minimap_y,Minimap_w,Minimap_h);
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
		std::map<gamemap::location,unit>::const_iterator i
		                                = units_.find(mouseoverHex_);
		if(i == units_.end() || fogged(i->first.x,i->first.y)) {
			i = units_.find(selectedHex_);
		}

		if(i != units_.end() && !fogged(i->first.x,i->first.y)) {
			draw_unit_details(mapx()+SideBarText_x,SideBarUnit_y,selectedHex_,
			                  i->second,unitDescriptionRect_,
			                  mapx()+SideBarText_x,SideBarUnitProfile_y);
		}

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

	SDL_Rect rect;
	rect.x = x;
	rect.y = y;
	rect.w = this->x() - x;

	const time_of_day& tod = timeofday_at(status_,units_,mouseoverHex_);

	SDL_Surface* const tod_surface = image::get_image(tod.image,image::UNSCALED);

	if(tod_surface != NULL) {
		//hardcoded values as to where the time of day image appears
		blit_surface(mapx() + TimeOfDay_x,TimeOfDay_y,tod_surface);
		update_rect(mapx() + TimeOfDay_x,TimeOfDay_y,tod_surface->w,tod_surface->h);
	}

	if(gameStatusRect_.w > 0) {
		SDL_Surface* const screen = screen_.getSurface();
		SDL_Surface* const background = image::get_image(RightSideTop,image::UNSCALED);

		if(background == NULL)
			return;

		SDL_Rect srcrect = gameStatusRect_;
		srcrect.x -= mapx();
		SDL_BlitSurface(background,&srcrect,screen,&gameStatusRect_);
		update_rect(gameStatusRect_);
	}

	std::stringstream details;

	if(team_valid()) {
		const team_data data = calculate_team_data(teams_[currentTeam_],
		                                           currentTeam_+1,units_);

		details << char(activeTeam_+1) << string_table["turn"] << ": "
		        << status_.turn() << "/" << status_.number_of_turns() << "\n"
		        << string_table["gold"] << ": " << data.gold << "\n"
				<< string_table["villages"] << ": " << data.villages << "\n"
				<< string_table["units"] << ": " << data.units << "\n"
		        << string_table["upkeep"] << ": " << data.upkeep << "\n"
				<< string_table["income"] << ": " << data.net_income << "\n";
	}

	if(map_.on_board(mouseoverHex_) && !shrouded(mouseoverHex_.x,mouseoverHex_.y)) {
		const gamemap::TERRAIN terrain = map_[mouseoverHex_.x][mouseoverHex_.y];
		std::string name = map_.terrain_name(terrain);
		std::string underlying_name = map_.underlying_terrain_name(terrain);

		if(terrain == gamemap::CASTLE &&
		   map_.is_starting_position(mouseoverHex_) != -1) {
			name = "keep";
		}

		const std::string& lang_name = string_table[name];
		const std::string& underlying_lang_name = string_table[underlying_name];

		const std::string& name1 = lang_name.empty() ? name : lang_name;
		const std::string& name2 = underlying_lang_name.empty() ?
		                              underlying_name : underlying_lang_name;

		if(name1 != name2)
			details << name1 << " (" << name2 << ")\n";
		else
			details << name1 << "\n";

		const int xhex = mouseoverHex_.x + 1;
		const int yhex = mouseoverHex_.y + 1;

		details << xhex << ", " << yhex;

		//we display the unit the mouse is over if it is over a unit
		//otherwise we display the unit that is selected
		std::map<gamemap::location,unit>::const_iterator unit_selected
		                                = units_.find(mouseoverHex_);
		if(unit_selected == units_.end()) {
			unit_selected = units_.find(selectedHex_);
		}

		if(unit_selected != units_.end() && map_.on_board(mouseoverHex_)) {
			const gamemap::TERRAIN terrain =
			                 map_[mouseoverHex_.x][mouseoverHex_.y];

			const unit& u = unit_selected->second;
			const int move_cost = u.movement_cost(map_,terrain);
			const int defense = 100 - u.defense_modifier(map_,terrain);

			if(move_cost > 10) {
				details << " (-)";
			} else {
				details << " (" << defense << "%," << move_cost << ")";
			}
		}

		details << "\n";

	} else {
		details << "\n";
	}

	SDL_Rect clipRect = screen_area();

	gameStatusRect_ = font::draw_text(this,clipRect,13,font::NORMAL_COLOUR,
	                                  details.str(),x,y);
	update_rect(gameStatusRect_);
}

void display::draw_unit_details(int x, int y, const gamemap::location& loc,
         const unit& u, SDL_Rect& description_rect, int profilex, int profiley,
         SDL_Rect* clip_rect)
{
	if(teams_.empty())
		return;

	tooltips::clear_tooltips(description_rect);

	SDL_Rect clipRect = clip_rect != NULL ? *clip_rect : screen_area();

	SDL_Surface* const background = image::get_image(RightSideTop,image::UNSCALED);
	SDL_Surface* const background_bot = image::get_image(RightSideBot,image::UNSCALED);

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
	   u.invisible(map_.underlying_terrain(map_[loc.x][loc.y]))) {
		status = "@" + string_table["invisible"];
	}

	if(u.has_flag("slowed")) {
		status = "#" + string_table["slowed"];
	}

	if(u.has_flag("poisoned")) {
		status = "#" + string_table["poisoned"];
	}

	std::stringstream details;
	details << "+" << u.description() << "\n"
	        << "+" << u.type().language_name()
			<< "\n-(" << string_table["level"] << " "
			<< u.type().level() << ")\n"
			<< status << "\n"
			<< unit_type::alignment_description(u.type().alignment())
			<< "\n"
			<< u.traits_description() << "\n";

	const std::vector<std::string>& abilities = u.type().abilities();
	for(std::vector<std::string>::const_iterator a = abilities.begin(); a != abilities.end(); ++a) {
		details << translate_string_default("ability_" + *a, *a) << "\n";
	}

	details << string_table["hp"] << ": " << u.hitpoints()
			<< "/" << u.max_hitpoints() << "\n"
			<< string_table["xp"] << ": ";
	
	if(u.type().advances_to().empty())
		details << u.experience() << "/-";
	else
		details << u.experience() << "/" << u.max_experience();
	
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

	description_rect =
	    font::draw_text(this,clipRect,13,font::NORMAL_COLOUR,
	                    details.str(),x,y);

	update_rect(description_rect);

	y += description_rect.h;

	SDL_Surface* const profile = image::get_image(u.type().image(),image::UNSCALED);

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
	SDL_Surface* const surface = getMinimap(w,h);
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

	const int wbox = static_cast<int>(xscaling*mapx()/(zoom_*0.75) - xscaling);
	const int hbox = static_cast<int>(yscaling*this->y()/zoom_ - yscaling);

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
		SDL_Surface* const image = getTerrain(*i,image::SCALED,-1,-1);
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

void display::draw_tile(int x, int y, SDL_Surface* unit_image,
                        double highlight_ratio, Pixel blend_with)

{
	if(updatesLocked_)
		return;

	const gamemap::location loc(x,y);
	int xpos = int(get_location_x(loc));
	int ypos = int(get_location_y(loc));

	if(xpos > mapx() || ypos > this->y())
		return;

	int xend = xpos + static_cast<int>(zoom_);
	int yend = int(get_location_y(gamemap::location(x,y+1)));

	if(xend < 0 || yend < 0)
		return;

	const gamemap::location ne_loc(loc.get_direction(
	                                  gamemap::location::NORTH_EAST));
	const gamemap::location se_loc(loc.get_direction(
	                                  gamemap::location::SOUTH_EAST));

	//assert(tiles_adjacent(loc,ne_loc));
	//assert(tiles_adjacent(loc,se_loc));

	const int ne_xpos = (int)get_location_x(ne_loc);
	const int ne_ypos = (int)get_location_y(ne_loc);
	const int se_xpos = (int)get_location_x(se_loc);
	const int se_ypos = (int)get_location_y(se_loc);

	const bool is_shrouded = shrouded(x,y);
	gamemap::TERRAIN terrain = gamemap::VOID_TERRAIN;

	if(map_.on_board(loc) && !is_shrouded) {
		terrain = map_[x][y];
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

	if(loc == mouseoverHex_ || loc == selectedHex_ &&
	   units_.count(gamemap::location(x,y)) == 1) {
		image_type = image::BRIGHTENED;
	}

	SDL_Surface* surface = getTerrain(terrain,image_type,x,y);

	if(surface == NULL) {
		std::cerr << "Could not get terrain surface\n";
		return;
	}

	std::vector<SDL_Surface*> overlaps;

	if(!is_shrouded) {
		SDL_Surface* const flag = getFlag(terrain,x,y);
		if(flag != NULL)
			overlaps.push_back(flag);

		const std::vector<SDL_Surface*>& adj = getAdjacentTerrain(x,y,image_type);

		overlaps.insert(overlaps.end(),adj.begin(),adj.end());

		typedef std::multimap<gamemap::location,std::string>::const_iterator
		        Itor;

		for(std::pair<Itor,Itor> overlays =
		    overlays_.equal_range(gamemap::location(x,y));
			overlays.first != overlays.second; ++overlays.first) {
			SDL_Surface* const overlay_surface =
			          image::get_image(overlays.first->second);

			if(overlay_surface != NULL) {
				overlaps.push_back(overlay_surface);
			}
		}
	}

	int ysrc = 0, xsrc = 0;
	if(ypos < 0) {
		ysrc -= ypos;
		ypos = 0;
	}

	if(xpos < 0) {
		xsrc -= xpos;
		xpos = 0;
	}

	if(xend >= mapx())
		xend = mapx();

	if(yend >= this->y())
		yend = this->y();

	if(xend < xpos || yend < ypos)
		return;

	update_rect(xpos,ypos,xend-xpos,yend-ypos);

	SDL_Surface* energy_image = NULL;

	double unit_energy = 0.0;

	const short energy_loss_colour = 0;
	short energy_colour = 0;

	const int max_energy = 80;
	double energy_size = 1.0;
	bool face_left = true;

	//see if there is a unit on this tile
	const unit_map::const_iterator it = units_.find(gamemap::location(x,y));
	if(it != units_.end()) {
		if(unit_image == NULL)
			unit_image = image::get_image(it->second.image());
		const int unit_move = it->second.movement_left();
		const int unit_total_move = it->second.total_movement();

		const char* energy_file = NULL;

		if(size_t(it->second.side()) != currentTeam_+1) {
			if(team_valid() &&
			   teams_[currentTeam_].is_enemy(it->second.side())) {
				energy_file = "enemy-energy.png";
			} else {
				energy_file = "ally-energy.png";
			}
		} else {
			if(activeTeam_ == currentTeam_ && unit_move == unit_total_move) {
				energy_file = "unmoved-energy.png";
			} else if(activeTeam_ == currentTeam_ && unit_can_move(loc,units_,map_,teams_)) {
				energy_file = "partmoved-energy.png";
			} else {
				energy_file = "moved-energy.png";
			}
		}

		energy_image = image::get_image(energy_file);
		unit_energy = double(it->second.hitpoints()) /
		              double(it->second.max_hitpoints());

		if(highlight_ratio == 1.0)
			highlight_ratio = it->second.alpha();

		if(it->second.invisible(map_.underlying_terrain(map_[x][y])) &&
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

	const int xpad = is_odd(surface->w);

	SDL_Surface* const dst = screen_.getSurface();

	Pixel grid_colour = SDL_MapRGB(dst->format,0,0,0);

	const bool show_unit_colour = preferences::show_side_colours() && it != units_.end();
	if(show_unit_colour) {
		const SDL_Color& colour = font::get_side_colour(it->second.side());
		grid_colour = SDL_MapRGB(dst->format,colour.r,colour.g,colour.b);
	}

	int j;
	for(j = ypos; j != yend; ++j) {
		const int yloc = ysrc+j-ypos;
		const int xoffset = abs(yloc - static_cast<int>(zoom_/2.0))/2;

		const int ne_yloc = ysrc+j-ne_ypos;
		const int ne_xoffset = abs(ne_yloc - static_cast<int>(zoom_/2.0))/2;

		const int se_yloc = ysrc+j-se_ypos;
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

		surface_lock srclock(surface);
		short* startsrc = srclock.pixels() + srcy*(surface->w+xpad) +
		                  maximum<int>(xoffset,xsrc);
		short* endsrc = startsrc + len;

		if(!(startsrc >= srclock.pixels() &&
		       endsrc <= srclock.pixels() + (surface->w+xpad)*surface->h)) {
			std::cerr << "CRITICAL ERROR: overwrite at " << __FILE__ << ","
			          << __LINE__ << "\n"
			          << "len: " << len << "\n"
			          << "width: " << surface->w << "\n"
			          << "x: " << x << "\n"
			          << "y: " << y << "\n";
		}

		surface_lock dstlock(dst);
		short* startdst = dstlock.pixels() + j*dst->w + xdst;
		std::copy(startsrc,endsrc,startdst);

		int extra = 0;

		//if the line didn't make it to the next hex, then fill in with the
		//last pixel up to the next hex
		if(ne_ypos > 0 && xdst + len < minoffset && len > 0) {
			extra = minimum(minoffset-(xdst + len),mapx()-(xdst+len));
			std::fill(startdst+len,startdst+len+extra,startsrc[len-1]);
		}

		//copy any overlapping tiles on
		for(std::vector<SDL_Surface*>::const_iterator ov = overlaps.begin();
		    ov != overlaps.end(); ++ov) {
			const int srcy = minimum<int>(yloc,(*ov)->h-1);
			const int w = (*ov)->w + is_odd((*ov)->w);

			surface_lock overlap_lock(*ov);
			short* beg = overlap_lock.pixels() +
			             srcy*w + (xoffset > xsrc ? xoffset:xsrc);
			short* end = beg + len;
			short* dst = startdst;

			while(beg != end) {
				if(*beg != 0) {
					*dst = *beg;
				}

				++dst;
				++beg;
			}

			//fill in any extra pixels on the end
			if(extra > 0 && len > 0 && beg[-1] != 0)
				std::fill(dst,dst+extra,beg[-1]);
		}

		if((grid_ || show_unit_colour) && startsrc < endsrc) {
			*startdst = grid_colour;
			*(startdst+len-1) = grid_colour;
			if(j == ypos || j == yend-1) {
				std::fill(startdst,startdst+len,grid_colour);
			}
		}
	}

	if(game_config::debug && debugHighlights_.count(gamemap::location(x,y))) {
		SDL_Surface* const cross = image::get_image("cross.png");
		if(cross != NULL)
			draw_unit(xpos-xsrc,ypos-ysrc,cross,face_left,false,
			          debugHighlights_[loc],0);
	}

	if(!is_shrouded) {
		draw_footstep(loc,xpos-xsrc,ypos-ysrc);
	}

	if(unit_image == NULL || energy_image == NULL || fogged(x,y))
		return;

	if(loc != hiddenUnit_) {
		if(draw_hit) {
			blend_with = hit_colour;
			highlight_ratio = 0.7;
		} else if(loc == advancingUnit_ && it != units_.end()) {
			//set the advancing colour to white if it's a non-chaotic unit,
			//otherwise black
			blend_with = it->second.type().alignment() == unit_type::CHAOTIC ?
			                                        0x0001 : 0xFFFF;
			highlight_ratio = advancingAmount_;
		}

		draw_unit(xpos-xsrc,ypos-ysrc,unit_image,face_left,false,
		          highlight_ratio,blend_with);
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

	static const std::string left_nw("misc/foot-left-nw.png");
	static const std::string left_n("misc/foot-left-n.png");
	static const std::string right_nw("misc/foot-right-nw.png");
	static const std::string right_n("misc/foot-right-n.png");

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

	SDL_Surface* const image = image::get_image(*image_str);
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
		static std::string str(1,'x');
		str[0] = '0' + route_.move_left + 1;
		const SDL_Rect& text_area =
		    font::draw_text(NULL,screen_area(),18,font::BUTTON_COLOUR,str,0,0);
		const int x = xloc + int(zoom_/2.0) - text_area.w/2;
		const int y = yloc + int(zoom_/2.0) - text_area.h/2;
		font::draw_text(this,screen_area(),18,font::BUTTON_COLOUR,str,x,y);
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

	const gamemap::TERRAIN current_terrain = map_.on_board(loc) ? map_[x][y]:0;

	gamemap::location adjacent[6];
	get_adjacent_tiles(loc,adjacent);
	int tiles[6];
	for(int i = 0; i != 6; ++i) {
		tiles[i] = map_.on_board(adjacent[i]) ?
				   (int)map_[adjacent[i].x][adjacent[i].y] :
				   (int)gamemap::VOID_TERRAIN;
	}

	const std::vector<gamemap::TERRAIN>& precedence =
	                                 map_.get_terrain_precedence();
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
					SDL_Surface* const new_surface = getTerrain(
					                    *terrain,image_type,x,y,stream.str());

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

	if(terrain == gamemap::CASTLE &&
	   map_.is_starting_position(gamemap::location(x,y)) != -1) {
		image = "terrain/keep";
	}

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

	if(x < 0)
		x = 0;

	if(y < 0)
		y = 0;

	//lines are padded to always fit on 4-byte boundaries, so see if there
	//is padding at the beginning of every line
	const int padding = is_odd(surface->w);
	const int surface_width = surface->w + padding;

	surface_lock srclock(surface);
	surface_lock dstlock(target);

	const short* src = srclock.pixels() + srcy*surface_width + srcx;
	short* dst = dstlock.pixels() + y*target->w + x;

	static const short transperant = 0;

	for(int i = 0; i != srch; ++i) {
		const short* s = src + i*surface_width + padding;
		const short* const end = s + srcw;
		short* d = dst + i*target->w;
		while(s != end) {
			if(*s != transperant) {
				*d = *s;
			}

			++s;
			++d;
		}
	}
}

SDL_Surface* display::getMinimap(int w, int h)
{
	if(minimap_ == NULL)
		minimap_ = image::getMinimap(w,h,map_,team_valid() ? &teams_[currentTeam_] : NULL);

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
	return static_cast<double>(loc.x)*zoom_*0.75 - xpos_;
}

double display::get_location_y(const gamemap::location& loc) const
{
	return static_cast<double>(loc.y)*zoom_ - ypos_ +
					         ((loc.x % 2) == 1 ? zoom_/2.0 : 0.0);
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
			SDL_Surface* const image = (unit_image == NULL) ?
			                            NULL : image::get_image(*unit_image);
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

			static const std::string default_missile("missile-n.png");
			static const std::string default_diag_missile("missile-ne.png");
			if(missile_image == NULL) {
				if(dir == attack_type::VERTICAL)
					missile_image = &default_missile;
				else
					missile_image = &default_diag_missile;
			}

			SDL_Surface* const img = image::get_image(*missile_image);
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
		if(xloc < side_threshhold) {
			scroll(xloc - side_threshhold,0.0);
		}

		if(yloc < side_threshhold) {
			scroll(0.0,yloc - side_threshhold);
		}

		if(xloc + zoom_ > this->mapx() - side_threshhold) {
			scroll(((xloc + zoom_) -
			        (this->mapx() - side_threshhold)),0.0);
		}

		if(yloc + zoom_ > this->y() - side_threshhold) {
			scroll(0.0,((yloc + zoom_) - (this->y() - side_threshhold)));
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

		SDL_Surface* const image = (unit_image == NULL) ?
		                           NULL : image::get_image(*unit_image);

		const double pos = double(i)/double(i < 0 ? begin_at : end_at);
		const int posx = int(pos*xsrc + (1.0-pos)*xdst) + xoffset;
		const int posy = int(pos*ysrc + (1.0-pos)*ydst);

		if(image != NULL && !hide)
			draw_unit(posx,posy,image,attacker.facing_left());

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

	const double nsteps = turbo() ? 3.0 : 10.0;
	const double xstep = (xdst - xsrc)/nsteps;
	const double ystep = (ydst - ysrc)/nsteps;

	const int time_between_frames = turbo() ? 2 : 10;
	int ticks = SDL_GetTicks();

	int skips = 0;

	for(double i = 0.0; i < nsteps; i += 1.0) {
		events::pump();

		//checking keys may have invalidated all images (if they have
		//zoomed in or out), so reget the image here
		SDL_Surface* const image = image::get_image(u.type().image());
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

		draw(false);
		draw_unit((int)xloc,(int)yloc,image,face_left);

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
                        double alpha, Pixel blendto)
{
	if(updatesLocked_) {
		return;
	}

	const int w = mapx()-1;
	const int h = this->y()-1;
	if(x > w || y > h)
		return;

	const int image_w = image->w + is_odd(image->w);

	SDL_Surface* const screen = screen_.getSurface();

	surface_lock srclock(image);
	const Pixel* src = srclock.pixels();

	const int endy = (y + image->h) < h ? (y + image->h) : h;
	const int endx = (x + image->w) < w ? (x + image->w) : w;
	if(endx < x)
		return;

	const int len = endx - x;

	if(y < 0) {
		//this is adding to src, since y is negative
		src -= image_w*y;
		y = 0;
		if(y >= endy)
			return;
	}

	int xoffset = 0;
	if(x < 0) {
		xoffset = -x;
		x = 0;
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

	SDL_Surface* const image = image::get_image("unmoved-energy.png");

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
	if(!invalidateAll_ && loc.valid()) {
		invalidated_.insert(loc);
	}
}

void display::invalidate_all()
{
	invalidateAll_ = true;
	invalidated_.clear();
	update_rect(0,0,mapx(),y());
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
