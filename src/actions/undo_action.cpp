/*
   Copyright (C) 2017 the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "actions/undo_action.hpp"
#include "game_board.hpp"
#include "scripting/game_lua_kernel.hpp"
#include "resources.hpp"
#include "variable.hpp" // vconfig
#include "game_data.hpp"
#include "units/unit.hpp"
#include "sound.hpp"

#include <cassert>
#include <iterator>
#include <algorithm>

namespace actions
{

undo_event::undo_event(const config& cmds, const game_events::queued_event& ctx)
	: commands(cmds)
	, data(ctx.data)
	, loc1(ctx.loc1)
	, loc2(ctx.loc2)
	, filter_loc1(ctx.loc1.filter_loc())
	, filter_loc2(ctx.loc2.filter_loc())
	, uid1(), uid2()
{
	unit_const_ptr u1 = ctx.loc1.get_unit(), u2 = ctx.loc2.get_unit();
	if(u1) {
		id1 = u1->id();
		uid1 = u1->underlying_id();
	}
	if(u2) {
		id2 = u2->id();
		uid2 = u2->underlying_id();
	}
}

undo_event::undo_event(const config& first, const config& second, const config& weapons, const config& cmds)
	: commands(cmds)
	, data(weapons)
	, loc1(first["x"], first["y"], wml_loc())
	, loc2(second["x"], second["y"], wml_loc())
	, filter_loc1(first["filter_x"], first["filter_y"], wml_loc())
	, filter_loc2(second["filter_x"], second["filter_y"], wml_loc())
	, uid1(first["underlying_id"])
	, uid2(second["underlying_id"])
	, id1(first["id"])
	, id2(second["id"])
{
}

undo_action::undo_action()
	: undo_action_base()
	, unit_id_diff(synced_context::get_unit_id_diff())
{
	auto& undo = synced_context::get_undo_commands();
	auto command_transformer = [](const std::pair<config, game_events::queued_event>& p) {
		return undo_event(p.first, p.second);
	};
	std::transform(undo.begin(), undo.end(), std::back_inserter(umc_commands_undo), command_transformer);
	undo.clear();
}

undo_action::undo_action(const config& cfg)
	: undo_action_base()
	, unit_id_diff(cfg["unit_id_diff"])
{
	read_event_vector(umc_commands_undo, cfg, "undo_actions");
}

namespace {
	unit_ptr get_unit(size_t uid, const std::string& id) {
		assert(resources::gameboard);
		auto iter = resources::gameboard->units().find(uid);
		if(!iter.valid() || iter->id() != id) {
			return nullptr;
		}
		return iter.get_shared_ptr();
	}
	void execute_event(const undo_event& e, std::string tag) {
		assert(resources::lua_kernel);
		assert(resources::gamedata);

		config::attribute_value& x1 = resources::gamedata->get_variable("x1");
		config::attribute_value& y1 = resources::gamedata->get_variable("y1");
		config::attribute_value& x2 = resources::gamedata->get_variable("x2");
		config::attribute_value& y2 = resources::gamedata->get_variable("y2");
		int oldx1 = x1, oldy1 = y1, oldx2 = x2, oldy2 = y2;
		x1 = e.filter_loc1.wml_x(); y1 = e.filter_loc1.wml_y();
		x2 = e.filter_loc2.wml_x(); y2 = e.filter_loc2.wml_y();

		std::unique_ptr<scoped_xy_unit> u1, u2;
		if(unit_ptr who = get_unit(e.uid1, e.id1)) {
			u1.reset(new scoped_xy_unit("unit", who->get_location(), resources::gameboard->units()));
		}
		if(unit_ptr who = get_unit(e.uid2, e.id2)) {
			u2.reset(new scoped_xy_unit("unit", who->get_location(), resources::gameboard->units()));
		}

		scoped_weapon_info w1("weapon", e.data.child("first"));
		scoped_weapon_info w2("second_weapon", e.data.child("second"));

		game_events::queued_event q(tag, "", map_location(x1, y1, wml_loc()), map_location(x2, y2, wml_loc()), e.data);
		resources::lua_kernel->run_wml_action("command", vconfig(e.commands), q);
		sound::commit_music_changes();

		x1 = oldx1; y1 = oldy1;
		x2 = oldx2; y2 = oldy2;
	}
}

void undo_action::execute_undo_umc_wml()
{
	for(const undo_event& e : umc_commands_undo)
	{
		execute_event(e, "undo");
	}
}


void undo_action::write(config & cfg) const
{
	cfg["unit_id_diff"] = unit_id_diff;
	write_event_vector(umc_commands_undo, cfg, "undo_actions");
	undo_action_base::write(cfg);
}

void undo_action::read_event_vector(event_vector& vec, const config& cfg, const std::string& tag)
{
	for(auto c : cfg.child_range(tag)) {
		vec.emplace_back(c.child("filter"), c.child("filter_second"), c.child("filter_weapons"), c.child("commands"));
	}
}

void undo_action::write_event_vector(const event_vector& vec, config& cfg, const std::string& tag)
{
	for(const auto& evt : vec)
	{
		config& entry = cfg.add_child(tag);
		config& first = entry.add_child("filter");
		config& second = entry.add_child("filter_second");
		entry.add_child("filter_weapons", evt.data);
		entry.add_child("command", evt.commands);
		// First location
		first["filter_x"] = evt.filter_loc1.wml_x();
		first["filter_y"] = evt.filter_loc1.wml_y();
		first["underlying_id"] = evt.uid1;
		first["id"] = evt.id1;
		first["x"] = evt.loc1.wml_x();
		first["y"] = evt.loc1.wml_y();
		// Second location
		second["filter_x"] = evt.filter_loc2.wml_x();
		second["filter_y"] = evt.filter_loc2.wml_y();
		second["underlying_id"] = evt.uid2;
		second["id"] = evt.id2;
		second["x"] = evt.loc2.wml_x();
		second["y"] = evt.loc2.wml_y();
	}
}

}
