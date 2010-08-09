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
 * @file
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

std::ostream &operator<<(std::ostream &s, attack_ptr attack)
{
	assert(attack);
	return attack->print(s);
}

std::ostream &operator<<(std::ostream &s, attack_const_ptr attack)
{
	assert(attack);
	return attack->print(s);
}

std::ostream& attack::print(std::ostream& s) const
{
	s << "Attack on (" << get_target_hex() << ") preceded by ";
	move::print(s);
	return s;
}

attack::attack(size_t team_index, const map_location& target_hex, int weapon_choice, const pathfind::marked_route& route,
		arrow_ptr arrow, fake_unit_ptr fake_unit)
	: move(team_index, route, arrow, fake_unit)
	, target_hex_(target_hex), weapon_choice_(weapon_choice)
{

}

attack::~attack()
{
	if(resources::screen)
	{
		//invalidate dest and target hex so attack indicator is properly cleared
		resources::screen->invalidate(get_dest_hex());
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

	LOG_WB << "Executing: " << shared_from_this() << "\n";

	events::mouse_handler const& mouse_handler = resources::controller->get_mouse_handler_base();

	std::set<map_location> adj_enemies = mouse_handler.get_adj_enemies(get_dest_hex(), side_number());

	if (execute_successful && route_->steps.size() >= 2)
	{
		if (!move::execute())
		{
			//Move didn't complete for some reason, so we're not at
			//the right hex to execute the attack.
			execute_successful = false;
		}
		//check if new enemies are now visible
		else if(mouse_handler.get_adj_enemies(get_dest_hex(), side_number()) != adj_enemies)
		{
			execute_successful = false; //ambush, interrupt attack
		}
	}

	if (execute_successful)
	{
		resources::controller->get_mouse_handler_base().attack_enemy(get_dest_hex(), get_target_hex(), weapon_choice_);
		//only path that returns execute_successful = true
	}
	return execute_successful;
}

void attack::draw_hex(const map_location& hex)
{
	if (hex == get_dest_hex() || hex == target_hex_) //draw attack indicator
	{
		//TODO: replace this by either the use of transparency + LAYER_ATTACK_INDICATOR,
		//or a dedicated layer
		const display::tdrawing_layer layer = display::LAYER_FOOTSTEPS;

		//calculate direction (valid for both hexes)
		std::string direction_text = map_location::write_direction(
				get_dest_hex().get_relative_dir(target_hex_));

		if (hex == get_dest_hex()) //add symbol to attacker hex
		{
			int xpos = resources::screen->get_location_x(get_dest_hex());
			int ypos = resources::screen->get_location_y(get_dest_hex());

			//TODO: Give the whiteboard its own copy of the attack indicator, so it can have a different look.
			resources::screen->drawing_buffer_add(layer, get_dest_hex(), display::tblit(xpos, ypos,
					image::get_image("misc/attack-indicator-src-" + direction_text + ".png", image::SCALED_TO_HEX)));
		}
		else if (hex == target_hex_) //add symbol to defender hex
		{
			int xpos = resources::screen->get_location_x(target_hex_);
			int ypos = resources::screen->get_location_y(target_hex_);

			resources::screen->drawing_buffer_add(layer, target_hex_, display::tblit(xpos, ypos,
					image::get_image("misc/attack-indicator-dst-" + direction_text + ".png", image::SCALED_TO_HEX)));
		}
	}
}

} // end namespace wb
