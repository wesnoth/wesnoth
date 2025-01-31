/*
	Copyright (C) 2017 - 2024
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

#include "actions/undo_action.hpp"
#include "game_board.hpp"
#include "log.hpp"                   // for LOG_STREAM, logger, etc
#include "scripting/game_lua_kernel.hpp"
#include "resources.hpp"
#include "variable.hpp" // vconfig
#include "game_data.hpp"
#include "units/unit.hpp"
#include "utils/ranges.hpp"
#include "sound.hpp"

#include <cassert>
#include <iterator>
#include <algorithm>

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)


namespace actions
{


undo_action_container::undo_action_container()
	: steps_()
	, unit_id_diff_(0)
{
}

bool undo_action_container::undo(int side)
{
	int last_unit_id = resources::gameboard->unit_id_manager().get_save_id();
	for(auto& p_step : steps_ | utils::views::reverse) {
		p_step->undo(side);
	}
	if(last_unit_id - unit_id_diff_ < 0) {
		ERR_NG << "Next unit id is below 0 after undoing";
	}
	resources::gameboard->unit_id_manager().set_save_id(last_unit_id - unit_id_diff_);
	return true;
}

void undo_action_container::add(t_step_ptr&& action)
{
	steps_.emplace_back(std::move(action));
}


void undo_action_container::read(const config& cfg)
{
	for(const config& step : cfg.child_range("step")) {
		auto& factory = get_factories()[step["type"]];
		add(factory(step));
	}
}
void undo_action_container::write(config& cfg)
{
	for(auto& p_step : steps_) {
		p_step->write(cfg.add_child("step"));
	}
}

undo_action_container::t_factory_map& undo_action_container::get_factories()
{
	static t_factory_map res;
	return res;
}






undo_event::undo_event(int fcn_idx, const config& args, const game_events::queued_event& ctx)
	: lua_idx(fcn_idx)
	, commands(args)
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
	, uid1(first["underlying_id"].to_size_t())
	, uid2(second["underlying_id"].to_size_t())
	, id1(first["id"])
	, id2(second["id"])
{
}

undo_event::undo_event(const config& cfg)
	: undo_event(cfg.child_or_empty("filter"),
		cfg.child_or_empty("filter_second"),
		cfg.child_or_empty("data"),
		cfg.child_or_empty("command"))
{
}


namespace
{
unit_ptr get_unit(std::size_t uid, const std::string& id)
{
	assert(resources::gameboard);
	auto iter = resources::gameboard->units().find(uid);
	if(!iter.valid() || iter->id() != id) {
		return nullptr;
	}
	return iter.get_shared_ptr();
}
} // namespace

bool undo_event::undo(int)
{
	undo_event& e = *this;
	std::string tag = "undo";
	assert(resources::lua_kernel);
	assert(resources::gamedata);

	config::attribute_value x1 = config::attribute_value::create(e.filter_loc1.wml_x());
	config::attribute_value y1 = config::attribute_value::create(e.filter_loc1.wml_y());
	config::attribute_value x2 = config::attribute_value::create(e.filter_loc2.wml_x());
	config::attribute_value y2 = config::attribute_value::create(e.filter_loc2.wml_y());
	std::swap(x1, resources::gamedata->get_variable("x1"));
	std::swap(y1, resources::gamedata->get_variable("y1"));
	std::swap(x2, resources::gamedata->get_variable("x2"));
	std::swap(y2, resources::gamedata->get_variable("y2"));

	std::unique_ptr<scoped_xy_unit> u1, u2;
	if(unit_ptr who = get_unit(e.uid1, e.id1)) {
		u1.reset(new scoped_xy_unit("unit", who->get_location(), resources::gameboard->units()));
	}
	if(unit_ptr who = get_unit(e.uid2, e.id2)) {
		u2.reset(new scoped_xy_unit("unit", who->get_location(), resources::gameboard->units()));
	}

	scoped_weapon_info w1("weapon", e.data.optional_child("first"));
	scoped_weapon_info w2("second_weapon", e.data.optional_child("second"));

	game_events::queued_event q(tag, "", map_location(x1, y1, wml_loc()), map_location(x2, y2, wml_loc()), e.data);
	if(e.lua_idx.has_value()) {
		resources::lua_kernel->run_wml_event(*e.lua_idx, vconfig(e.commands), q);
	} else {
		resources::lua_kernel->run_wml_action("command", vconfig(e.commands), q);
	}
	sound::commit_music_changes();

	std::swap(x1, resources::gamedata->get_variable("x1"));
	std::swap(y1, resources::gamedata->get_variable("y1"));
	std::swap(x2, resources::gamedata->get_variable("x2"));
	std::swap(y2, resources::gamedata->get_variable("y2"));
	return true;
}

void undo_event::write(config& cfg) const
{
	undo_action::write(cfg);
	auto& evt = *this;
	if(evt.lua_idx.has_value()) {
		// TODO: Log warning that this cannot be serialized
		return;
	}
	config& entry = cfg;
	config& first = entry.add_child("filter");
	config& second = entry.add_child("filter_second");
	entry.add_child("data", evt.data);
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


static auto red_undo_event = undo_action_container::subaction_factory<undo_event>();

} // namespace actions
