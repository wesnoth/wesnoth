/* $Id$ */
/*
   Copyright (C) 2003-5 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "display.hpp"
#include "halo.hpp"
#include "image.hpp"
#include "preferences.hpp"
#include "sdl_utils.hpp"
#include "util.hpp"
#include "video.hpp"
#include "wassert.hpp"
#include "serialization/string_utils.hpp"

#include <algorithm>
#include <map>

namespace halo
{

namespace {
display* disp = NULL;

class effect
{
public:
	effect(int xpos, int ypos, const animated<std::string>::anim_description& img,
			const gamemap::location& loc, ORIENTATION, bool infinite);

	void set_location(int x, int y);

	bool render();
	void unrender();

	bool expired() const { return images_.animation_finished(); }
	bool need_update() const { return images_.need_update(); }
	bool does_change() const { return !images_.does_not_change(); }
	bool on_location(const std::set<gamemap::location>& locations) const; 

	void add_overlay_location(std::set<gamemap::location>& locations);
private:

	const std::string& current_image() { return images_.get_current_frame(); }
	void rezoom();

	animated<std::string> images_;

	std::string current_image_;

	ORIENTATION orientation_;

	int origx_, origy_, x_, y_;
	double origzoom_, zoom_;
	surface surf_, buffer_;
	SDL_Rect rect_;

	// the location of the center of the halo
	gamemap::location loc_;

	// all location over which the halo lies
	std::vector<gamemap::location> overlayed_hexes_;
};

std::map<int, effect> haloes;
int halo_id = 1;

// Upon unrendering an invalidation list is send. All haloes in that area and the 
// other invalidated haloes are stored in this set. Then there'll be tested which
// haloes overlap and they're also stored in this set. 
std::set<int> invalidated_haloes; 

// A newly added halo will be added to this list, these haloes don't need to
// be unrendered but do not to be rendered regardless which tiles are invalidated.
// These haloes will stay in this set until there're really rendered.
// (rendering won't happen if for example the halo is offscreen)
std::set<int> new_haloes;

// Upon deleting a halo isn't deleted but added to this set, upon unrendering the
// image is unrendered and deleted.
std::set<int> deleted_haloes;

// Haloes that have an animation or expiration time need to be checked every frame
// and are stored in this set.
std::set<int> changing_haloes;
	
effect::effect(int xpos, int ypos, const animated<std::string>::anim_description& img,
	const gamemap::location& loc, ORIENTATION orientation, bool infinite) :
		images_(img), orientation_(orientation), origx_(xpos), origy_(ypos), 
		x_(xpos), y_(ypos), origzoom_(disp->zoom()), zoom_(disp->zoom()), 
		surf_(NULL), buffer_(NULL), rect_(empty_rect), loc_(loc)
{
	wassert(disp != NULL);

	set_location(xpos,ypos);

	images_.start_animation(0,infinite);

	if(!images_.animation_finished()) {
		images_.update_last_draw_time();
	}

	current_image_ = "";
	rezoom();
}

void effect::set_location(int x, int y)
{
	const gamemap::location zero_loc(0,0);
	x_ = origx_ = x - disp->get_location_x(zero_loc);
	y_ = origy_ = y - disp->get_location_y(zero_loc);
	origzoom_ = disp->zoom();

	if(zoom_ != origzoom_) {
		rezoom();
	}
}

void effect::rezoom()
{
	zoom_ = disp->zoom();
	x_ = int((origx_*zoom_)/origzoom_);
	y_ = int((origy_*zoom_)/origzoom_);

	surf_.assign(image::get_image(current_image_,image::UNSCALED));
	if(surf_ != NULL && (orientation_ == HREVERSE || orientation_ == HVREVERSE)) {
		surf_.assign(image::reverse_image(surf_));
	}
	if(surf_ != NULL && (orientation_ == VREVERSE || orientation_ == HVREVERSE)) {
		surf_.assign(flop_surface(surf_));
	}

	if(surf_ != NULL && zoom_ != 1.0) {
		surf_.assign(scale_surface(surf_,int(surf_->w*zoom_),int(surf_->h*zoom_)));
	}
}

bool effect::render()
{
	if(disp == NULL) {
		return false;
	}
	
	if(loc_.x != -1 && loc_.y != -1 && disp->shrouded(loc_.x, loc_.y)) {
		return false;
	}

	images_.update_last_draw_time();
	const std::string& img = current_image();
	if(surf_ == NULL || zoom_ != disp->zoom() || current_image_ != img) {
		current_image_ = img;
		rezoom();
	}

	if(surf_ == NULL) {
		return false;
	}

	const gamemap::location zero_loc(0,0);
	const int screenx = disp->get_location_x(zero_loc);
	const int screeny = disp->get_location_y(zero_loc);

	const int xpos = x_ + screenx - surf_->w/2;
	const int ypos = y_ + screeny - surf_->h/2;

	SDL_Rect rect = {xpos,ypos,surf_->w,surf_->h};
	rect_ = rect;
	SDL_Rect clip_rect = disp->map_area();
	if(rects_overlap(rect,clip_rect) == false) {
		buffer_.assign(NULL);
		return false;
	}

	// if rendered the first time need to detemine the area affected, if a halo
	// changes size it's not updated.
	if(overlayed_hexes_.empty()) {
		gamemap::location topleft, bottomright;
		disp->get_rect_hex_bounds(rect, topleft, bottomright);
		for (int x = topleft.x; x <= bottomright.x; ++x) {
			for (int y = topleft.y; y <= bottomright.y; ++y) {
				overlayed_hexes_.push_back(gamemap::location(x, y));
			}
		}
	}

	surface const screen = disp->video().getSurface();

	const clip_rect_setter clip_setter(screen,clip_rect);
	if(buffer_ == NULL || buffer_->w != rect.w || buffer_->h != rect.h) {
		SDL_Rect rect = rect_;
		buffer_.assign(get_surface_portion(screen,rect));
	} else {
		SDL_Rect rect = rect_;
		SDL_BlitSurface(screen,&rect,buffer_,NULL);
	}

	SDL_BlitSurface(surf_,NULL,screen,&rect);

	update_rect(rect_);

	return true;
}

void effect::unrender()
{
	if(buffer_ == NULL) {
		return;
	}

	surface const screen = disp->video().getSurface();

	SDL_Rect clip_rect = disp->map_area();
	const clip_rect_setter clip_setter(screen,clip_rect);

	// due to scrolling the location of the rendered halo might have changed; recalculate
	const gamemap::location zero_loc(0,0);
	const int screenx = disp->get_location_x(zero_loc);
	const int screeny = disp->get_location_y(zero_loc);

	const int xpos = x_ + screenx - surf_->w/2;
	const int ypos = y_ + screeny - surf_->h/2;

	SDL_Rect rect = {xpos,ypos,surf_->w,surf_->h};
	SDL_BlitSurface(buffer_,NULL,screen,&rect);
	update_rect(rect_);
}

bool effect::on_location(const std::set<gamemap::location>& locations) const 
{
	for(std::vector<gamemap::location>::const_iterator itor = overlayed_hexes_.begin();
			itor != overlayed_hexes_.end(); ++itor) {
		if(locations.find(*itor) != locations.end()) {
			return true;
		}
	}
	return false;
}

void effect::add_overlay_location(std::set<gamemap::location>& locations)
{
	for(std::vector<gamemap::location>::const_iterator itor = overlayed_hexes_.begin();
			itor != overlayed_hexes_.end(); ++itor) {

		locations.insert(*itor);
	}
}

} // namespace

manager::manager(display& screen) : old(disp)
{
	disp = &screen;
}

manager::~manager()
{
	haloes.clear();
	disp = old;
}

int add(int x, int y, const std::string& image, const gamemap::location& loc,
		ORIENTATION orientation, bool infinite)
{
	const int id = halo_id++;
	animated<std::string>::anim_description image_vector;
	std::vector<std::string> items = utils::split(image);
	std::vector<std::string>::const_iterator itor = items.begin();
	for(; itor != items.end(); ++itor) {
		const std::vector<std::string>& items = utils::split(*itor, ':');
		std::string str;
		int time;

		if(items.size() > 1) {
			str = items.front();
			time = atoi(items.back().c_str());
		} else {
			str = *itor;
			time = 100;
		}
		image_vector.push_back(animated<std::string>::frame_description(time,std::string(str)));

	}
	haloes.insert(std::pair<int,effect>(id,effect(x,y,image_vector,loc,orientation,infinite)));
	new_haloes.insert(id);
	if(haloes.find(id)->second.does_change() || !infinite) {
		changing_haloes.insert(id);
	}
	return id;
}

void set_location(int handle, int x, int y)
{
	const std::map<int,effect>::iterator itor = haloes.find(handle);
	if(itor != haloes.end()) {
		itor->second.set_location(x,y);
	}
}

void remove(int handle)
{
	deleted_haloes.insert(handle);
}

void unrender(std::set<gamemap::location> invalidated_locations)
{
	wassert(invalidated_haloes.size() == 0);
	if(preferences::show_haloes() == false || haloes.size() == 0) {
		return;
	}

	// remove expired haloes
	std::map<int, effect>::iterator itor = haloes.begin();
	for(; itor != haloes.end(); ++itor ) {
		if(itor->second.expired()) {
			deleted_haloes.insert(itor->first);
		}
	}

	// add the haloes marked for deletion to the invalidation set
	std::set<int>::const_iterator set_itor = deleted_haloes.begin();
	for(;set_itor != deleted_haloes.end(); ++set_itor) {
		invalidated_haloes.insert(*set_itor);
		haloes.find(*set_itor)->second.add_overlay_location(invalidated_locations);
	}
	
	// test the multi-frame haloes whether they need an update
	for(set_itor = changing_haloes.begin();
			set_itor != changing_haloes.end(); ++set_itor) {
		if(haloes.find(*set_itor)->second.need_update()) {
			invalidated_haloes.insert(*set_itor);
			haloes.find(*set_itor)->second.add_overlay_location(invalidated_locations);
		}
	}

	// find all halo's in a the invalidated area
	size_t halo_count;

	// repeat until of haloes in the invalidated area didn't change (including none found)
	// or all exisiting haloes are found
	do {
		halo_count = invalidated_haloes.size();
		for(itor = haloes.begin(); itor != haloes.end(); ++itor) {
			// test all haloes not yet in the set which match one of the locations
			if(invalidated_haloes.find(itor->first) == invalidated_haloes.end() && 
					itor->second.on_location(invalidated_locations)) {
				
				// if found add all locations which the halo invalidates
				// and add it to the set
				itor->second.add_overlay_location(invalidated_locations);
				invalidated_haloes.insert(itor->first);
			}
		}
	} while (halo_count != invalidated_haloes.size() && halo_count != haloes.size());

	if(halo_count == 0) {
		return;
	}
	
	// render the haloes iterate through all the haloes and invalidate if in set
	for(std::map<int, effect>::reverse_iterator ritor = haloes.rbegin(); ritor != haloes.rend(); ++ritor) {
		if(invalidated_haloes.find(ritor->first) != invalidated_haloes.end()) {
			ritor->second.unrender();
		}
	}

	// realy delete the haloes marked for deletion
	for(set_itor = deleted_haloes.begin(); set_itor != deleted_haloes.end(); ++set_itor) {
		// it can happen a delete halo hasn't been rendered yet, invalidate them as well
		new_haloes.erase(*set_itor);
		
		changing_haloes.erase(*set_itor);
		invalidated_haloes.erase(*set_itor);
		haloes.erase(*set_itor);
	}

	deleted_haloes.clear();
}

void render()
{

	if(preferences::show_haloes() == false || haloes.size() == 0 ||
			(new_haloes.size() == 0 && invalidated_haloes.size() == 0)) {
		return;
	}

	// keep track of not rendered new images they have to be kept scheduled for rendering
	// otherwise the invalidation area is never properly set
	std::set<int> unrendered_new_haloes;

	// render the haloes iterate through all the haloes and draw if in either set
	for(std::map<int, effect>::iterator itor = haloes.begin(); 
			itor != haloes.end(); ++itor) {

		if(new_haloes.find(itor->first) != new_haloes.end() && 
				! itor->second.render()) {
			
			unrendered_new_haloes.insert(itor->first);
		} else if(invalidated_haloes.find(itor->first) != invalidated_haloes.end()) {
			itor->second.render();
		}
	}

	invalidated_haloes.clear();
	new_haloes = unrendered_new_haloes;
}

}
