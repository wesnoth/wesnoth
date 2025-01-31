/*
	Copyright (C) 2003 - 2024
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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
#include "draw.hpp"
#include "draw_manager.hpp"
#include "halo.hpp"
#include "log.hpp"
#include "serialization/string_utils.hpp"
#include "sdl/rect.hpp"
#include "sdl/texture.hpp"

static lg::log_domain log_halo("halo");
#define ERR_HL LOG_STREAM(err, log_halo)
#define WRN_HL LOG_STREAM(warn, log_halo)
#define LOG_HL LOG_STREAM(info, log_halo)
#define DBG_HL LOG_STREAM(debug, log_halo)

namespace halo
{

class halo_impl
{

	class effect
	{
	public:
		effect(
			int xpos, int ypos,
			const animated<image::locator>::anim_description& img,
			const map_location& loc, ORIENTATION, bool infinite
		);

		void set_location(int x, int y);
		rect get_draw_location();

		/** Whether the halo is currently visible */
		bool visible();

		void queue_undraw();
		void queue_redraw();
		void update();
		bool render();

		bool expired()     const { return !images_.cycles() && images_.animation_finished(); }
		bool need_update() const { return images_.need_update(); }
		bool does_change() const { return !images_.does_not_change(); }
		bool on_location(const std::set<map_location>& locations) const;
		bool location_not_known() const;

	private:

		const image::locator& current_image() const { return images_.get_current_frame(); }

		animated<image::locator> images_;

		ORIENTATION orientation_;

		// The mid-point of the halo in pixels relative to the absolute top-left of the map, in screen coordinates.
		// Yes it's just as ridiculous as it sounds...
		// TODO: make this something sane. Like a floating-point map location.
		point abs_mid_ = {0, 0};

		// The current halo image frame
		texture tex_ = {};
		// The current location where the halo will be drawn on the screen
		rect screen_loc_ = {};
		// The last drawn location
		rect last_draw_loc_ = {};
		// The display zoom level, cached so we can compensate when it changes.
		double cached_zoom_ = 1.0;

		// The map location the halo is attached to, if any
		map_location map_loc_ = {-1, -1};

		display* disp = nullptr;
	};

	std::map<int, effect> haloes;
	int halo_id;

	/**
	 * Upon unrendering, an invalidation list is send. All haloes in that area and
	 * the other invalidated haloes are stored in this set. Then there'll be
	 * tested which haloes overlap and they're also stored in this set.
	 */
	std::set<int> invalidated_haloes;

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

	explicit halo_impl() :
		haloes(),
		halo_id(1),
		invalidated_haloes(),
		deleted_haloes(),
		changing_haloes()
	{}


	int add(int x, int y, const std::string& image, const map_location& loc,
			ORIENTATION orientation=NORMAL, bool infinite=true);

	/** Set the position of an existing haloing effect, according to its handle. */
	void set_location(int handle, int x, int y);

	/** Remove the halo with the given handle. */
	void remove(int handle);

	void update();

	/** Render all halos overlapping the given region */
	void render(const rect&);

}; //end halo_impl

halo_impl::effect::effect(int xpos, int ypos,
		const animated<image::locator>::anim_description& img,
		const map_location& loc, ORIENTATION orientation, bool infinite) :
	images_(img),
	orientation_(orientation),
	map_loc_(loc),
	disp(display::get_singleton())
{
	assert(disp != nullptr);

	cached_zoom_ = disp->get_zoom_factor();

	set_location(xpos, ypos);

	images_.start_animation(std::chrono::milliseconds{0}, infinite);

	update();
}

void halo_impl::effect::set_location(int x, int y)
{
	point new_center = point{x, y} - disp->get_location(map_location::ZERO());
	if(new_center != abs_mid_) {
		DBG_HL << "setting halo location " << new_center;
		abs_mid_ = new_center;
	}
}

rect halo_impl::effect::get_draw_location()
{
	return screen_loc_;
}


/** Update the current location, animation frame, etc. */
void halo_impl::effect::update()
{
	double zf = disp->get_zoom_factor();

	if(map_loc_.x != -1 && map_loc_.y != -1) {
		// If the halo is attached to a particular map location,
		// make sure it stays attached.
		auto [x, y] = disp->get_location_rect(map_loc_).center();
		set_location(x, y);
	} else {
		// It would be good to attach to a position within a hex,
		// or persistently to an item or unit. That's not the case,
		// so we use some horrible hacks to compensate for zoom changes.
		if(cached_zoom_ != zf) {
			abs_mid_.x *= zf / cached_zoom_;
			abs_mid_.y *= zf / cached_zoom_;
			cached_zoom_ = zf;
		}
	}

	// Load texture for current animation frame
	tex_ = image::get_texture(current_image());
	if(!tex_) {
		ERR_HL << "no texture found for current halo animation frame";
		screen_loc_ = {};
		return;
	}

	// Update draw location
	int w(tex_.w() * disp->get_zoom_factor());
	int h(tex_.h() * disp->get_zoom_factor());

	const auto [zero_x, zero_y] = disp->get_location(map_location::ZERO());

	const int xpos = zero_x + abs_mid_.x - w/2;
	const int ypos = zero_y + abs_mid_.y - h/2;

	screen_loc_ = {xpos, ypos, w, h};

	// Queue display updates if position has changed
	if(screen_loc_ != last_draw_loc_) {
		queue_undraw();
		queue_redraw();
		last_draw_loc_ = screen_loc_;
	}
}

bool halo_impl::effect::visible()
{
	// Source is shrouded
	// The halo will be completely obscured here, even if it would
	// technically be large enough to peek out of the shroud.
	if(map_loc_.x != -1 && map_loc_.y != -1 && disp->shrouded(map_loc_)) {
		return false;
	}

	// Halo is completely off screen
	if(!screen_loc_.overlaps(disp->map_outside_area())) {
		return false;
	}

	return true;
}

bool halo_impl::effect::render()
{
	// This should only be set if we actually draw something
	last_draw_loc_ = {};

	// Update animation frame, even if we didn't actually draw it
	images_.update_last_draw_time();

	if(!visible()) {
		return false;
	}

	// Make sure we clip to the map area
	auto clipper = draw::reduce_clip(disp->map_outside_area());

	DBG_HL << "drawing halo at " << screen_loc_;

	if (orientation_ == NORMAL) {
		draw::blit(tex_, screen_loc_);
	} else {
		draw::flipped(tex_, screen_loc_,
			orientation_ == HREVERSE || orientation_ == HVREVERSE,
			orientation_ == VREVERSE || orientation_ == HVREVERSE);
	}

	last_draw_loc_ = screen_loc_;

	return true;
}

void halo_impl::effect::queue_undraw()
{
	if(!last_draw_loc_.overlaps(disp->map_outside_area())) {
		return;
	}
	DBG_HL << "queueing halo undraw at " << last_draw_loc_;
	draw_manager::invalidate_region(last_draw_loc_);
}

void halo_impl::effect::queue_redraw()
{
	if(!visible()) {
		return;
	}
	DBG_HL << "queueing halo redraw at " << screen_loc_;
	draw_manager::invalidate_region(screen_loc_);
}



/*************/
/* halo_impl */
/*************/


int halo_impl::add(int x, int y, const std::string& image, const map_location& loc,
		ORIENTATION orientation, bool infinite)
{
	const int id = halo_id++;
	DBG_HL << "adding halo " << id;
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
			} catch(const std::invalid_argument&) {
				ERR_HL << "Invalid time value found when constructing halo: " << sub_items.back();
			}
		}
		image_vector.push_back(animated<image::locator>::frame_description(time,image::locator(str)));

	}
	haloes.emplace(id, effect(x, y, image_vector, loc, orientation, infinite));
	invalidated_haloes.insert(id);
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

void halo_impl::update()
{
	if(haloes.empty()) {
		return;
	}

	// Mark expired haloes for removal
	for(auto& [id, effect] : haloes) {
		if(effect.expired()) {
			DBG_HL << "expiring halo " << id;
			deleted_haloes.insert(id);
		}
	}
	// Make sure deleted halos get undrawn
	for(int id : deleted_haloes) {
		DBG_HL << "invalidating deleted halo " << id;
		haloes.at(id).queue_undraw();
	}
	// Remove deleted halos
	for(int id : deleted_haloes) {
		DBG_HL << "deleting halo " << id;
		changing_haloes.erase(id);
		haloes.erase(id);
	}
	deleted_haloes.clear();

	// Update the location and animation frame of the remaining halos
	for(auto& [id, halo] : haloes) { (void)id;
		halo.update();
	}

	// Invalidate any animated halos which need updating
	for(int id : changing_haloes) {
		auto& halo = haloes.at(id);
		if(halo.need_update() && halo.visible()) {
			DBG_HL << "invalidating changed halo " << id;
			halo.queue_redraw();
		}
	}
}

void halo_impl::render(const rect& region)
{
	if(haloes.empty()) {
		return;
	}

	for(auto& [id, effect] : haloes) {
		if(region.overlaps(effect.get_draw_location())) {
			DBG_HL << "drawing intersected halo " << id;
			effect.render();
		}
	}
}



/*****************/
/* halo::manager */
/*****************/


manager::manager() : impl_(new halo_impl())
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

void manager::update()
{
	impl_->update();
}

void manager::render(const rect& r)
{
	impl_->render(r);
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

	if(man) {
		man->remove(id_);
	}
}

} //end namespace halo
