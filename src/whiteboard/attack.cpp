/* $Id$ */
/*
 Copyright (C) 2010 by Gabriel Morin <gabrielmorin (at) gmail (dot) com>
 Part of the Battle for Wesnoth Project http://www.wesnoth.org

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 2
 or at your option any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY.

 See the COPYING file for more details.
 */

/**
 * @file attack.cpp
 */

#include "attack.hpp"

#include "visitor.hpp"

#include "arrow.hpp"
#include "play_controller.hpp"
#include "resources.hpp"
#include "unit.hpp"
#include "unit_map.hpp"

namespace wb
{

std::ostream &operator<<(std::ostream &s, wb::attack const& attack)
{
//	s << "Attack for unit " << attack->get_unit().name() << " [" << attack->get_unit().underlying_id() << "] "
//			<< "moving from (" << attack->get_source_hex() << ") to (" << attack->get_dest_hex() << ") and attacking "
//			<< attack->get_target_hex();

	return attack.print(s);
}

std::ostream& attack::print(std::ostream& s) const
{
	s << static_cast<wb::move>(*this) << " and attacking (" << get_target_hex() << ")";
	return s;
}

attack::attack(const map_location& target_hex, int weapon_choice, const map_location& source_hex, const map_location& dest_hex,
		arrow_ptr arrow, fake_unit_ptr fake_unit)
	: move(source_hex, dest_hex, arrow, fake_unit)
	, target_hex_(target_hex), weapon_choice_(weapon_choice)
{

}

attack::~attack()
{
	if(resources::screen)
	{
		//invalidate dest and target hex so attack indicator is properly cleared
		resources::screen->invalidate(dest_hex_);
		resources::screen->invalidate(target_hex_);
	}
}

void attack::accept(visitor& v)
{
	v.visit_attack(boost::static_pointer_cast<attack>(shared_from_this()));
}

bool attack::execute()
{
	bool execute_successful = true;

	if (!valid_)
		execute_successful = false;

	LOG_WB << "Executing: " << *this << "\n";

	if (execute_successful && arrow_->get_path().size() >= 2)
	{
		if (!move::execute())
		{
			//Move didn't complete for some reason, so we're not at
			//the right hex to execute the attack.
			execute_successful = false;
		}
	}

	if (execute_successful)
	{
		resources::controller->get_mouse_handler_base().attack_enemy(dest_hex_, target_hex_, weapon_choice_);
		//only path that returns execute_successful = true
	}
	return execute_successful;
}

void attack::draw_hex(const map_location& hex)
{
	if (hex == dest_hex_ || hex == target_hex_) //draw attack indicator
	{
		//TODO: replace this by either the use of transparency + LAYER_ATTACK_INDICATOR,
		//or a dedicated layer
		const display::tdrawing_layer layer = display::LAYER_FOOTSTEPS;

		//calculate direction (valid for both hexes)
		std::string direction_text = map_location::write_direction(
				dest_hex_.get_relative_dir(target_hex_));

		if (hex == dest_hex_) //add symbol to attacker hex
		{
			int xpos = resources::screen->get_location_x(dest_hex_);
			int ypos = resources::screen->get_location_y(dest_hex_);

			//TODO: Give the whiteboard its own copy of the attack indicator, so it can have a different look.
			resources::screen->drawing_buffer_add(layer, dest_hex_, display::tblit(xpos, ypos,
					image::get_image("misc/attack-indicator-src-" + direction_text + ".png", image::UNMASKED)));
		}
		else if (hex == target_hex_) //add symbol to defender hex
		{
			int xpos = resources::screen->get_location_x(target_hex_);
			int ypos = resources::screen->get_location_y(target_hex_);

			resources::screen->drawing_buffer_add(layer, target_hex_, display::tblit(xpos, ypos,
					image::get_image("misc/attack-indicator-dst-" + direction_text + ".png", image::UNMASKED)));
		}
	}
}

} // end namespace wb
