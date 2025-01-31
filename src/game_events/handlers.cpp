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
 * The structure that tracks WML event handlers.
 * (Typically, handlers are defined by [event] tags.)
 */

#include "game_events/handlers.hpp"
#include "game_events/conditional_wml.hpp"
#include "game_events/pump.hpp"
#include "game_events/manager_impl.hpp" // for standardize_name

#include "formula/callable_objects.hpp"
#include "formula/formula.hpp"
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
#include "units/unit.hpp"
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
	, has_preloaded_(false)
	, event_ref_(0)
	, priority_(0.0)
	, args_()
	, filters_()
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

void event_handler::write_config(config &cfg, bool include_nonserializable) const
{
	if(disabled_) {
		WRN_NG << "Tried to serialize disabled event, skipping";
		return;
	}
	static const char* log_append_preload = " - this will not break saves since it was registered during or before preload\n";
	static const char* log_append_postload = " - this will break saves because it was registered after preload\n";
	if(is_lua_) {
		if(include_nonserializable) {
			cfg["nonserializable"] = true;
			cfg.add_child("lua")["code"] = "<function>";
		} else {
			static const char* log = "Skipping serialization of an event with action bound to Lua code";
			if(has_preloaded_){
				WRN_NG << log << log_append_postload;
				lg::log_to_chat() << log << log_append_postload;
			} else {
				LOG_NG << log << log_append_preload;
			}
			return;
		}
	}
	if(!std::all_of(filters_.begin(), filters_.end(), std::mem_fn(&event_filter::can_serialize))) {
		if(include_nonserializable) {
			cfg["nonserializable"] = true;
		} else {
			static const char* log = "Skipping serialization of an event with filter bound to Lua code";
			if(has_preloaded_) {
				WRN_NG << log << log_append_postload;
				lg::log_to_chat() << log << log_append_postload;
			} else {
				LOG_NG << log << log_append_preload;
			}
			return;
		}
	}
	if(!types_.empty()) cfg["name"] = types_;
	if(!id_.empty()) cfg["id"] = id_;
	cfg["first_time_only"] = first_time_only_;
	cfg["priority"] = priority_;
	for(const auto& filter : filters_) {
		filter->serialize(cfg);
	}
	cfg.append(args_);
}

void event_filter::serialize(config&) const
{
	WRN_NG << "Tried to serialize an event with a filter that cannot be serialized!";
}

bool event_filter::can_serialize() const
{
	return false;
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
	bool can_serialize() const override
	{
		return true;
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
	bool can_serialize() const override
	{
		return true;
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
	bool can_serialize() const override
	{
		return true;
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
		const auto& loc_d = first_ ? event_info.loc2 : event_info.loc1;
		auto unit_a = units.find(loc);
		auto unit_d = units.find(loc_d);
		if(unit_a != units.end() && loc.matches_unit(unit_a)) {
			const auto u = unit_a->shared_from_this();
			auto temp_weapon = event_info.data.optional_child(first_ ? "first" : "second");
			if(temp_weapon){
				const_attack_ptr attack = std::make_shared<const attack_type>(*temp_weapon);
				if(unit_d != units.end() && loc_d.matches_unit(unit_d)) {
					const auto opp = unit_d->shared_from_this();
					auto temp_other_weapon = event_info.data.optional_child(!first_ ? "first" : "second");
					const_attack_ptr second_attack = temp_other_weapon ? std::make_shared<const attack_type>(*temp_other_weapon) : nullptr;
					auto ctx = attack->specials_context(u, opp, loc, loc_d, first_, second_attack);
					utils::optional<decltype(ctx)> opp_ctx;
					if(second_attack){
						opp_ctx.emplace(second_attack->specials_context(opp, u, loc_d, loc, !first_, attack));
					}
					return swf_.empty() || attack->matches_filter(swf_.get_parsed_config());
				} else {
					auto ctx = attack->specials_context(u, loc, first_);
					return swf_.empty() || attack->matches_filter(swf_.get_parsed_config());
				}
			}
		}
		return false;
	}
	void serialize(config& cfg) const override
	{
		cfg.add_child(first_ ? "filter_attack" : "filter_second_attack", swf_.get_config());
	}
	bool can_serialize() const override
	{
		return true;
	}
private:
	vconfig swf_;
	bool first_;
};

struct filter_formula : public event_filter {
	filter_formula(const std::string& formula) : formula_(formula) {}
	bool operator()(const queued_event& event_info) const override
	{
		wfl::gamestate_callable gs;
		wfl::event_callable evt(event_info);
		wfl::formula_callable_with_backup data(evt, gs);
		return formula_.evaluate(data).as_bool();
	}
	void serialize(config& cfg) const override
	{
		std::string code = formula_.str();
		if(cfg.has_attribute("filter_formula")) {
			// This will probably never happen in practice, but handle it just in case it somehow can
			code = "(" + cfg["filter_formula"].str() + ") and (" + code + ")";
		}
		cfg["filter_formula"] = code;
	}
	bool can_serialize() const override
	{
		return true;
	}
private:
	wfl::formula formula_;
};

static std::unique_ptr<event_filter> make_filter(const std::string& key, const vconfig& contents)
{
	if(key == "filter_condition") {
		return std::make_unique<filter_condition>(contents);
	} else if(key == "filter_side") {
		return std::make_unique<filter_side>(contents);
	} else if(key == "filter") {
		return std::make_unique<filter_unit>(contents, true);
	} else if(key == "filter_attack") {
		return std::make_unique<filter_attack>(contents, true);
	} else if(key == "filter_second") {
		return std::make_unique<filter_unit>(contents, false);
	} else if(key == "filter_second_attack") {
		return std::make_unique<filter_attack>(contents, false);
	}
	return nullptr;
}

/**
 * This is a dynamic wrapper for any filter type, specified via [insert_tag].
 * It loads the filter contents from a variable and forwards it to the appropriate filter class.
 */
struct filter_dynamic : public event_filter {
	filter_dynamic(const std::string& tag, const std::string& var) : tag_(tag), var_(var) {}
	bool operator()(const queued_event& event_info) const override
	{
		variable_access_const variable(var_, resources::gamedata->get_variables());
		if(!variable.exists_as_container()) return false;
		if(auto filter = make_filter(tag_, vconfig(variable.as_container()))) {
			return (*filter)(event_info);
		}
		return false;
	}
	void serialize(config& cfg) const override
	{
		auto tag = cfg.add_child("insert_tag");
		tag["name"] = tag_;
		tag["variable"] = var_;
	}
	bool can_serialize() const override
	{
		return true;
	}
private:
	std::string tag_, var_;
};

void event_handler::read_filters(const config &cfg)
{
	for(const auto [filter_key, filter_cfg] : cfg.all_children_view()) {
		vconfig vcfg(filter_cfg);
		if(auto filter_ptr = make_filter(filter_key, vcfg)) {
			add_filter(std::move(filter_ptr));
		} else if(filter_key == "insert_tag" && make_filter(vcfg["name"], vconfig::empty_vconfig())) {
			add_filter(std::make_unique<filter_dynamic>(vcfg["name"], vcfg["variable"]));
		}
	}
	if(cfg.has_attribute("filter_formula")) {
		add_filter(std::make_unique<filter_formula>(cfg["filter_formula"]));
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

void event_handler::set_event_ref(int idx, bool has_preloaded)
{
	event_ref_ = idx;
	is_lua_ = true;
	has_preloaded_ = has_preloaded;
}


} // end namespace game_events
