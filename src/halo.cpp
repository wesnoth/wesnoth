/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
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

#include "halo.hpp"
#include "animated.hpp"
#include "display.hpp"
#include "log.hpp"
#include "preferences/game.hpp"
#include "serialization/string_utils.hpp"

#include <iostream>

static lg::log_domain log_display("display");
#define ERR_DP LOG_STREAM(err, log_display)

namespace halo
{
class halo_impl
{
public:
	halo_impl()
		: halos()
		, halo_id(1)
	{
	}

	int add(int x,
			int y,
			const std::string& image,
			const map_location& loc,
			ORIENTATION orientation = NORMAL,
			bool infinite = true);

	/** Set the position of an existing haloing effect, according to its handle. */
	void set_location(int handle, int x, int y);

	/** Remove the halo with the given handle. */
	void remove(int handle);

	/**
	 * Render halos.
	 *
	 * Which halos are rendered is determined by invalidated_locations and the
	 * internal state in the control sets (in halo.cpp).
	 */
	void render();

private:
	/** Encapsulates the drawing of a single halo effect. */
	class effect
	{
	public:
		effect(int xpos,
				int ypos,
				const animated<image::locator>::anim_description& img,
				const map_location& loc,
				ORIENTATION,
				bool infinite);

		void set_location(int x, int y);

		bool render();

		bool expired() const
		{
			return !images_.cycles() && images_.animation_finished();
		}

		bool need_update() const
		{
			return images_.need_update();
		}

		bool does_change() const
		{
			return !images_.does_not_change();
		}

	private:
		const image::locator& current_image() const
		{
			return images_.get_current_frame();
		}

		animated<image::locator> images_;

		ORIENTATION orientation_;

		int x_, y_;

		texture texture_;

		/** The location of the center of the halo. */
		map_location loc_;

		display* disp;
	};

	std::map<int, effect> halos;

	int halo_id;
};

halo_impl::effect::effect(
		int xpos,
		int ypos,
		const animated<image::locator>::anim_description& img,
		const map_location& loc,
		ORIENTATION orientation,
		bool infinite)
	: images_(img)
	, orientation_(orientation)
	, x_(0)
	, y_(0)
	, texture_(nullptr)
	, loc_(loc)
	, disp(display::get_singleton())
{
	assert(disp != nullptr);

	set_location(xpos, ypos);

	images_.start_animation(0, infinite);
}

void halo_impl::effect::set_location(int x, int y)
{
	int new_x = x - disp->get_location_x(map_location::ZERO());
	int new_y = y - disp->get_location_y(map_location::ZERO());

	if(new_x != x_ || new_y != y_) {
		x_ = new_x;
		y_ = new_y;
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
			// This glitch is most visible on [item] halos.
			// This workaround always recalculates the location of the halo
			// (item halos have a location parameter to hide them under the shroud)
			// and reapplies that location.
			// It might be optimized by storing and comparing the zoom value.
			set_location(
				disp->get_location_x(loc_) + disp->hex_size() / 2,
				disp->get_location_y(loc_) + disp->hex_size() / 2
			);
		}
	}

	images_.update_last_draw_time();

	texture_ = image::get_texture(current_image() /*, image::SCALED_TO_ZOOM*/);
	if(texture_ == nullptr) {
		return false;
	}

	const int screenx = disp->get_location_x(map_location::ZERO());
	const int screeny = disp->get_location_y(map_location::ZERO());

	texture::info t_info = texture_.get_info();

	const double zoom_factor = disp->get_zoom_factor();

	const int xpos = x_ + screenx - (t_info.w / 2) * zoom_factor;
	const int ypos = y_ + screeny - (t_info.h / 2) * zoom_factor;

	// TODO: decide if I need this
	// SDL_Rect clip_rect = disp->map_outside_area();
	// const clip_rect_setter clip_setter(screen, &clip_rect);

	const bool h_flip = orientation_ == HREVERSE || orientation_ == HVREVERSE;
	const bool v_flip = orientation_ == VREVERSE || orientation_ == HVREVERSE;

	disp->render_scaled_to_zoom(texture_, xpos, ypos, h_flip, v_flip);

	return true;
}


//
// HALO IMPL =========================================================================
//

int halo_impl::add(
		int x, int y, const std::string& image, const map_location& loc, ORIENTATION orientation, bool infinite)
{
	const int id = halo_id++;

	animated<image::locator>::anim_description image_vector;

	for(const std::string& item : utils::square_parenthetical_split(image, ',')) {
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

		image_vector.emplace_back(time, image::locator(str));
	}

	halos.emplace(id, effect(x, y, image_vector, loc, orientation, infinite));

	return id;
}

void halo_impl::set_location(int handle, int x, int y)
{
	const auto itor = halos.find(handle);
	if(itor != halos.end()) {
		itor->second.set_location(x, y);
	}
}

void halo_impl::remove(int handle)
{
	// Silently ignore invalid halos. This happens when Wesnoth is being terminated as well.
	if(handle == NO_HALO || halos.find(handle) == halos.end()) {
		return;
	}

	auto itor = halos.find(handle);
	if(itor != halos.end()) {
		halos.erase(itor);
	}
}

void halo_impl::render()
{
	if(!preferences::show_haloes() || halos.empty()) {
		return;
	}

	//
	// Clean up expired halos.
	//
	for(auto itor = halos.begin(); itor != halos.end(); /* Handle increment in loop*/) {
		if(itor->second.expired()) {
			halos.erase(itor++);
		} else {
			++itor;
		}
	}

	//
	// Render the halos.
	//
	for(auto& halo : halos) {
		halo.second.render();
	}
}


//
// HALO MANAGER =========================================================================
//

manager::manager()
	: impl_(new halo_impl())
{
}

handle manager::add(
		int x, int y, const std::string& image, const map_location& loc, ORIENTATION orientation, bool infinite)
{
	int new_halo = impl_->add(x, y, image, loc, orientation, infinite);
	return handle(new halo_record(new_halo, impl_));
}

void manager::set_location(const handle& h, int x, int y)
{
	impl_->set_location(h->id_, x, y);
}

void manager::remove(const handle& h)
{
	impl_->remove(h->id_);
	h->id_ = NO_HALO;
}

void manager::render()
{
	impl_->render();
}


//
// HALO RECORD =========================================================================
//

halo_record::halo_record()
	: id_(NO_HALO) // halo::NO_HALO
	, my_manager_()
{
}

halo_record::halo_record(int id, const std::shared_ptr<halo_impl>& my_manager)
	: id_(id)
	, my_manager_(my_manager)
{
}

halo_record::~halo_record()
{
	if(!valid()) {
		return;
	}

	std::shared_ptr<halo_impl> man = my_manager_.lock();

	if(man) {
		try {
			man->remove(id_);
		} catch(std::exception& e) {
			std::cerr << "Caught an exception in halo::halo_record destructor: \n" << e.what() << std::endl;
		} catch(...) {
		}
	}
}

} // end namespace halo
