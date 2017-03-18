/*
   Copyright (C) 2003 - 2016 by David White <dave@whitevine.net>
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
 * Maintain halo-effects for units and items.
 * Examples: white mage, lighthouse.
 */

#include "animated.hpp"
#include "display.hpp"
#include "game_preferences.hpp"
#include "halo.hpp"
#include "log.hpp"
#include "serialization/string_utils.hpp"

#include <iostream>

static lg::log_domain log_display("display");
#define ERR_DP LOG_STREAM(err, log_display)

namespace halo
{

class halo_impl
{

class effect
{
public:
	effect(display * screen, int xpos, int ypos, const animated<image::locator>::anim_description& img,
			const map_location& loc, ORIENTATION, bool infinite);

	void set_location(int x, int y);

	bool render();
	void unrender();

	bool expired()     const { return !images_.cycles() && images_.animation_finished(); }
	bool need_update() const { return images_.need_update(); }
	bool does_change() const { return !images_.does_not_change(); }
	bool on_location(const std::set<map_location>& locations) const;
	bool location_not_known() const;

	void add_overlay_location(std::set<map_location>& locations);
private:

	const image::locator& current_image() const { return images_.get_current_frame(); }

	animated<image::locator> images_;

	ORIENTATION orientation_;

	int x_, y_;
	surface surf_, buffer_;
	SDL_Rect rect_;

	/** The location of the center of the halo. */
	map_location loc_;

	/** All locations over which the halo lies. */
	std::vector<map_location> overlayed_hexes_;

	display * disp;
};

display* disp;

std::map<int, effect> haloes;
int halo_id;

/**
 * Upon unrendering, an invalidation list is send. All haloes in that area and
 * the other invalidated haloes are stored in this set. Then there'll be
 * tested which haloes overlap and they're also stored in this set.
 */
std::set<int> invalidated_haloes;

/**
 * A newly added halo will be added to this list, these haloes don't need to be
 * unrendered but do not to be rendered regardless which tiles are invalidated.
 * These haloes will stay in this set until there're really rendered (rendering
 * won't happen if for example the halo is offscreen).
 */
std::set<int> new_haloes;

/**
 * Upon deleting, a halo isn't deleted but added to this set, upon unrendering
 * the image is unrendered and deleted.
 */
std::set<int> deleted_haloes;

/**
 * Haloes that have an animation or expiration time need to be checked every
 * frame and are stored in this set.
 */
std::set<int> changing_haloes;

public:
/**
 * impl's of exposed functions
 */

explicit halo_impl(display & screen) :
	disp(&screen),
	haloes(),
	halo_id(1),
	invalidated_haloes(),
	new_haloes(),
	deleted_haloes(),
	changing_haloes()
{}


int add(int x, int y, const std::string& image, const map_location& loc,
		ORIENTATION orientation=NORMAL, bool infinite=true);

/** Set the position of an existing haloing effect, according to its handle. */
void set_location(int handle, int x, int y);

/** Remove the halo with the given handle. */
void remove(int handle);

/**
 * Render and unrender haloes.
 *
 * Which haloes are rendered is determined by invalidated_locations and the
 * internal state in the control sets (in halo.cpp).
 */
void unrender(std::set<map_location> invalidated_locations);
void render();

}; //end halo_impl

halo_impl::effect::effect(display * screen, int xpos, int ypos, const animated<image::locator>::anim_description& img,
		const map_location& loc, ORIENTATION orientation, bool infinite) :
	images_(img),
	orientation_(orientation),
	x_(0),
	y_(0),
	surf_(nullptr),
	buffer_(nullptr),
	rect_(sdl::empty_rect),
	loc_(loc),
	overlayed_hexes_(),
	disp(screen)
{
	assert(disp != nullptr);

	set_location(xpos,ypos);

	images_.start_animation(0,infinite);

}

void halo_impl::effect::set_location(int x, int y)
{
	int new_x = x - disp->get_location_x(map_location::ZERO());
	int new_y = y - disp->get_location_y(map_location::ZERO());
	if (new_x != x_ || new_y != y_) {
		unrender();
		x_ = new_x;
		y_ = new_y;
		buffer_.assign(nullptr);
		overlayed_hexes_.clear();
	}
}

bool halo_impl::effect::render()
{
	if(disp == nullptr) {
		return false;
	}

	if(loc_.x != -1 && loc_.y != -1) {
		if(disp->shrouded(loc_)) {
			return false;
		} else {
			// The location of a halo is an x,y value and not a map location.
			// This means when a map is zoomed, the halo's won't move,
			// This glitch is most visible on [item] haloes.
			// This workaround always recalculates the location of the halo
			// (item haloes have a location parameter to hide them under the shroud)
			// and reapplies that location.
			// It might be optimized by storing and comparing the zoom value.
			set_location(
				disp->get_location_x(loc_) + disp->hex_size() / 2,
				disp->get_location_y(loc_) + disp->hex_size() / 2);
		}
	}

	images_.update_last_draw_time();
	surf_.assign(image::get_image(current_image(),image::SCALED_TO_ZOOM));
	if(surf_ == nullptr) {
		return false;
	}
	if(orientation_ == HREVERSE || orientation_ == HVREVERSE) {
		surf_.assign(image::reverse_image(surf_));
	}
	if(orientation_ == VREVERSE || orientation_ == HVREVERSE) {
		surf_.assign(flop_surface(surf_));
	}

	const int screenx = disp->get_location_x(map_location::ZERO());
	const int screeny = disp->get_location_y(map_location::ZERO());

	const int xpos = x_ + screenx - surf_->w/2;
	const int ypos = y_ + screeny - surf_->h/2;

	SDL_Rect rect = sdl::create_rect(xpos, ypos, surf_->w, surf_->h);
	rect_ = rect;
	SDL_Rect clip_rect = disp->map_outside_area();

	// If rendered the first time, need to determine the area affected.
	// If a halo changes size, it is not updated.
	if(location_not_known()) {
		display::rect_of_hexes hexes = disp->hexes_under_rect(rect);
		display::rect_of_hexes::iterator i = hexes.begin(), end = hexes.end();
		for (;i != end; ++i) {
			overlayed_hexes_.push_back(*i);
		}
	}

	if(sdl::rects_overlap(rect,clip_rect) == false) {
		buffer_.assign(nullptr);
		return false;
	}

	surface& screen = disp->get_screen_surface();

	const clip_rect_setter clip_setter(screen, &clip_rect);
	if(buffer_ == nullptr || buffer_->w != rect.w || buffer_->h != rect.h) {
		SDL_Rect rect2 = rect_;
		buffer_.assign(get_surface_portion(screen,rect2));
	} else {
		SDL_Rect rect2 = rect_;
		sdl_copy_portion(screen,&rect2,buffer_,nullptr);
	}

	sdl_blit(surf_,nullptr,screen,&rect);

	return true;
}

void halo_impl::effect::unrender()
{
	if (!surf_ || !buffer_) {
		return;
	}

	// Shrouded haloes are never rendered unless shroud has been re-placed; in
	// that case, unrendering causes the hidden terrain (and previous halo
	// frame, when dealing with animated halos) to glitch through shroud. We
	// don't need to unrender them because shroud paints over the underlying
	// area anyway.
	if (loc_.x != -1 && loc_.y != -1 && disp->shrouded(loc_)) {
		return;
	}

	surface& screen = disp->get_screen_surface();

	SDL_Rect clip_rect = disp->map_outside_area();
	const clip_rect_setter clip_setter(screen, &clip_rect);

	// Due to scrolling, the location of the rendered halo
	// might have changed; recalculate
	const int screenx = disp->get_location_x(map_location::ZERO());
	const int screeny = disp->get_location_y(map_location::ZERO());

	const int xpos = x_ + screenx - surf_->w/2;
	const int ypos = y_ + screeny - surf_->h/2;

	SDL_Rect rect = sdl::create_rect(xpos, ypos, surf_->w, surf_->h);
	sdl_blit(buffer_,nullptr,screen,&rect);
}

bool halo_impl::effect::on_location(const std::set<map_location>& locations) const
{
	for(std::vector<map_location>::const_iterator itor = overlayed_hexes_.begin();
			itor != overlayed_hexes_.end(); ++itor) {
		if(locations.find(*itor) != locations.end()) {
			return true;
		}
	}
	return false;
}

bool halo_impl::effect::location_not_known() const
{
	return overlayed_hexes_.empty();
}

void halo_impl::effect::add_overlay_location(std::set<map_location>& locations)
{
	for(std::vector<map_location>::const_iterator itor = overlayed_hexes_.begin();
			itor != overlayed_hexes_.end(); ++itor) {

		locations.insert(*itor);
	}
}

// End halo_impl::effect impl's

int halo_impl::add(int x, int y, const std::string& image, const map_location& loc,
		ORIENTATION orientation, bool infinite)
{
	const int id = halo_id++;
	animated<image::locator>::anim_description image_vector;
	std::vector<std::string> items = utils::square_parenthetical_split(image, ',');

	for(const std::string& item : items) {
		const std::vector<std::string>& sub_items = utils::split(item, ':');
		std::string str = item;
		int time = 100;

		if(sub_items.size() > 1) {
			str = sub_items.front();
			try {
				time = std::stoi(sub_items.back());
			} catch(std::invalid_argument) {
				ERR_DP << "Invalid time value found when constructing halo: " << sub_items.back() << "\n";
			}
		}
		image_vector.push_back(animated<image::locator>::frame_description(time,image::locator(str)));

	}
	haloes.insert(std::pair<int,effect>(id,effect(disp,x,y,image_vector,loc,orientation,infinite)));
	new_haloes.insert(id);
	if(haloes.find(id)->second.does_change() || !infinite) {
		changing_haloes.insert(id);
	}
	return id;
}

void halo_impl::set_location(int handle, int x, int y)
{
	const std::map<int,effect>::iterator itor = haloes.find(handle);
	if(itor != haloes.end()) {
		itor->second.set_location(x,y);
	}
}

void halo_impl::remove(int handle)
{
	// Silently ignore invalid haloes.
	// This happens when Wesnoth is being terminated as well.
	if(handle == NO_HALO || haloes.find(handle) == haloes.end())  {
		return;
	}

	deleted_haloes.insert(handle);
}

void halo_impl::unrender(std::set<map_location> invalidated_locations)
{
	if(preferences::show_haloes() == false || haloes.empty()) {
		return;
	}
	//assert(invalidated_haloes.empty());

	// Remove expired haloes
	std::map<int, effect>::iterator itor = haloes.begin();
	for(; itor != haloes.end(); ++itor ) {
		if(itor->second.expired()) {
			deleted_haloes.insert(itor->first);
		}
	}

	// Add the haloes marked for deletion to the invalidation set
	std::set<int>::const_iterator set_itor = deleted_haloes.begin();
	for(;set_itor != deleted_haloes.end(); ++set_itor) {
		invalidated_haloes.insert(*set_itor);
		haloes.find(*set_itor)->second.add_overlay_location(invalidated_locations);
	}

	// Test the multi-frame haloes whether they need an update
	for(set_itor = changing_haloes.begin();
			set_itor != changing_haloes.end(); ++set_itor) {
		if(haloes.find(*set_itor)->second.need_update()) {
			invalidated_haloes.insert(*set_itor);
			haloes.find(*set_itor)->second.add_overlay_location(invalidated_locations);
		}
	}

	// Find all halo's in a the invalidated area
	size_t halo_count;

	// Repeat until set of haloes in the invalidated area didn't change
	// (including none found) or all existing haloes are found.
	do {
		halo_count = invalidated_haloes.size();
		for(itor = haloes.begin(); itor != haloes.end(); ++itor) {
			// Test all haloes not yet in the set
			// which match one of the locations
			if(invalidated_haloes.find(itor->first) == invalidated_haloes.end() &&
					(itor->second.location_not_known() ||
					itor->second.on_location(invalidated_locations))) {

				// If found, add all locations which the halo invalidates,
				// and add it to the set
				itor->second.add_overlay_location(invalidated_locations);
				invalidated_haloes.insert(itor->first);
			}
		}
	} while (halo_count != invalidated_haloes.size() && halo_count != haloes.size());

	if(halo_count == 0) {
		return;
	}

	// Render the haloes:
	// iterate through all the haloes and invalidate if in set
	for(std::map<int, effect>::reverse_iterator ritor = haloes.rbegin(); ritor != haloes.rend(); ++ritor) {
		if(invalidated_haloes.find(ritor->first) != invalidated_haloes.end()) {
			ritor->second.unrender();
		}
	}

	// Really delete the haloes marked for deletion
	for(set_itor = deleted_haloes.begin(); set_itor != deleted_haloes.end(); ++set_itor) {
		// It can happen a deleted halo hasn't been rendered yet, invalidate them as well
		new_haloes.erase(*set_itor);

		changing_haloes.erase(*set_itor);
		invalidated_haloes.erase(*set_itor);
		haloes.erase(*set_itor);
	}

	deleted_haloes.clear();
}

void halo_impl::render()
{
	if(preferences::show_haloes() == false || haloes.empty() ||
			(new_haloes.empty() && invalidated_haloes.empty())) {
		return;
	}

	// Keep track of not rendered new images they have to be kept scheduled
	// for rendering otherwise the invalidation area is never properly set
	std::set<int> unrendered_new_haloes;

	// Render the haloes:
	// iterate through all the haloes and draw if in either set
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

// end halo_impl implementations

// begin halo::manager

manager::manager(display& screen) : impl_(new halo_impl(screen))
{}

handle manager::add(int x, int y, const std::string& image, const map_location& loc,
		ORIENTATION orientation, bool infinite)
{
	int new_halo = impl_->add(x,y,image, loc, orientation, infinite);
	return handle(new halo_record(new_halo, impl_));
}

/** Set the position of an existing haloing effect, according to its handle. */
void manager::set_location(const handle & h, int x, int y)
{
	impl_->set_location(h->id_,x,y);
}

/** Remove the halo with the given handle. */
void manager::remove(const handle & h)
{
	impl_->remove(h->id_);
	h->id_ = NO_HALO;
}

/**
 * Render and unrender haloes.
 *
 * Which haloes are rendered is determined by invalidated_locations and the
 * internal state in the control sets (in halo.cpp).
 */
void manager::unrender(std::set<map_location> invalidated_locations)
{
	impl_->unrender(invalidated_locations);
}

void manager::render()
{
	impl_->render();
}

// end halo::manager implementation


/**
 * halo::halo_record implementation
 */

halo_record::halo_record() :
	id_(NO_HALO), //halo::NO_HALO
	my_manager_()
{}

halo_record::halo_record(int id, const std::shared_ptr<halo_impl> & my_manager) :
	id_(id),
	my_manager_(my_manager)
{}

halo_record::~halo_record()
{
	if (!valid()) return;

	std::shared_ptr<halo_impl> man = my_manager_.lock();

	if (man) {
		try {
			man->remove(id_);
		} catch (std::exception & e) {
			std::cerr << "Caught an exception in halo::halo_record destructor: \n" << e.what() << std::endl;
		} catch (...) {}
	}
}

} //end namespace halo
