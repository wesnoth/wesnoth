/*
	Copyright (C) 2003 - 2022
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
 * The structure that tracks WML event handlers.
 * (Typically, handlers are defined by [event] tags.)
 */

#include "game_events/handlers.hpp"
#include "game_events/conditional_wml.hpp"
#include "game_events/pump.hpp"
#include "game_events/manager_impl.hpp" // for standardize_name

#include "formula/string_utils.hpp"
#include "game_board.hpp"
#include "game_data.hpp"
#include "log.hpp"
#include "play_controller.hpp"
#include "resources.hpp"
#include "scripting/game_lua_kernel.hpp"
#include "side_filter.hpp"
#include "sound.hpp"
#include "units/filter.hpp"
#include "variable.hpp"

static lg::log_domain log_engine("engine");
#define DBG_NG LOG_STREAM(debug, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)
#define WRN_NG LOG_STREAM(warn, log_engine)

static lg::log_domain log_event_handler("event_handler");
#define DBG_EH LOG_STREAM(debug, log_event_handler)

// This file is in the game_events namespace.
namespace game_events
{
/* ** event_handler ** */

event_handler::event_handler(const std::string& types, const std::string& id)
	: first_time_only_(true)
	, is_menu_item_(false)
	, disabled_(false)
	, is_lua_(false)
	, id_(id)
	, types_(types)
{}

std::vector<std::string> event_handler::names(const variable_set* vars) const
{
	std::string names = types_;

	// Do some standardization on the name field.
	// Split the name field and standardize each one individually.
	// This ensures they're all valid for by-name lookup.
	std::vector<std::string> standardized_names;
	for(std::string single_name : utils::split(names)) {
		if(utils::might_contain_variables(single_name)) {
			if(!vars) {
				// If we don't have gamedata, we can't interpolate variables, so there's
				// no way the name will match. Move on to the next one in that case.
				continue;
			}
			single_name = utils::interpolate_variables_into_string(single_name, *vars);
		}
		// Variable interpolation could've introduced additional commas, so split again.
		for(const std::string& subname : utils::split(single_name)) {
			standardized_names.emplace_back(event_handlers::standardize_name(subname));
		}
	}

	return standardized_names;
}

bool event_handler::empty() const
{
	return args_.empty();
}

void event_handler::disable()
{
	assert(!disabled_ && "Trying to disable a disabled event. Shouldn't happen!");
	disabled_ = true;
}

void event_handler::handle_event(const queued_event& event_info, game_lua_kernel& lk)
{
	if(disabled_) {
		return;
	}

	if(is_menu_item_) {
		DBG_NG << "menu item " << id_ << " will now invoke the following command(s):\n" << args_;
	}

	if(first_time_only_) {
		disable();
	}

	lk.run_wml_event(event_ref_, vconfig(args_, false), event_info);
	sound::commit_music_changes();
}

bool event_handler::filter_event(const queued_event& ev) const
{
	return std::all_of(filters_.begin(), filters_.end(), [&ev](const auto& filter) {
		return (*filter)(ev);
	});
}

void event_handler::write_config(config &cfg) const
{
	if(disabled_) {
		WRN_NG << "Tried to serialize disabled event, skipping";
		return;
	}
	if(is_lua_) {
		WRN_NG << "Skipping serialization of an event bound to Lua code";
		return;
	}
	if(!types_.empty()) cfg["name"] = types_;
	if(!id_.empty()) cfg["id"] = id_;
	cfg["first_time_only"] = first_time_only_;
	for(const auto& filter : filters_) {
		filter->serialize(cfg);
	}
	cfg.append(args_);
}

void event_filter::serialize(config&) const
{
	WRN_NG << "Tried to serialize an event with a filter that cannot be serialized!";
}

struct filter_condition : public event_filter {
	filter_condition(const vconfig& cfg) : cfg_(cfg.make_safe()) {}
	bool operator()(const queued_event&) const override
	{
		return conditional_passed(cfg_);
	}
	void serialize(config& cfg) const override
	{
		cfg.add_child("filter_condition", cfg_.get_config());
	}
private:
	vconfig cfg_;
};

struct filter_side : public event_filter {
	filter_side(const vconfig& cfg) : ssf_(cfg.make_safe(), &resources::controller->gamestate()) {}
	bool operator()(const queued_event&) const override
	{
		return ssf_.match(resources::controller->current_side());
	}
	void serialize(config& cfg) const override
	{
		cfg.add_child("filter_side", ssf_.get_config());
	}
private:
	side_filter ssf_;
};

struct filter_unit : public event_filter {
	filter_unit(const vconfig& cfg, bool first) : suf_(cfg.make_safe()), first_(first) {}
	bool operator()(const queued_event& event_info) const override
	{
		const auto& loc = first_ ? event_info.loc1 : event_info.loc2;
		auto unit = resources::gameboard->units().find(loc);
		return loc.matches_unit_filter(unit, suf_);
	}
	void serialize(config& cfg) const override
	{
		cfg.add_child(first_ ? "filter" : "filter_second", suf_.to_config());
	}
private:
	unit_filter suf_;
	bool first_;
};

struct filter_attack : public event_filter {
	filter_attack(const vconfig& cfg, bool first) : swf_(cfg.make_safe()), first_(first) {}
	bool operator()(const queued_event& event_info) const override
	{
		const unit_map& units = resources::gameboard->units();
		const auto& loc = first_ ? event_info.loc1 : event_info.loc2;
		auto unit = units.find(loc);
		if(unit != units.end() && loc.matches_unit(unit)) {
			const config& attack = event_info.data.child(first_ ? "first" : "second");
			return swf_.empty() || matches_special_filter(attack, swf_);
		}
		return false;
	}
	void serialize(config& cfg) const override
	{
		cfg.add_child(first_ ? "filter_attack" : "filter_second_attack", swf_.get_config());
	}
private:
	vconfig swf_;
	bool first_;
};

void event_handler::read_filters(const config &cfg)
{
	for(auto filter : cfg.all_children_range()) {
		vconfig vcfg(filter.cfg);
		if(filter.key == "filter_condition") {
			add_filter(std::make_unique<filter_condition>(vcfg));
		} else if(filter.key == "filter_side") {
			add_filter(std::make_unique<filter_side>(vcfg));
		} else if(filter.key == "filter") {
			add_filter(std::make_unique<filter_unit>(vcfg, true));
		} else if(filter.key == "filter_attack") {
			add_filter(std::make_unique<filter_attack>(vcfg, true));
		} else if(filter.key == "filter_second") {
			add_filter(std::make_unique<filter_unit>(vcfg, false));
		} else if(filter.key == "filter_second_attack") {
			add_filter(std::make_unique<filter_attack>(vcfg, false));
		}
	}
}

void event_handler::add_filter(std::unique_ptr<event_filter>&& filter)
{
	filters_.push_back(std::move(filter));
}

void event_handler::register_wml_event(game_lua_kernel &lk)
{
	event_ref_ = lk.save_wml_event();
}

void event_handler::set_event_ref(int idx)
{
	event_ref_ = idx;
	is_lua_ = true;
}


} // end namespace game_events
