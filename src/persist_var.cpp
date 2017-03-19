/*
   Copyright (C) 2010 - 2017 by Jody Northup
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "game_data.hpp"
#include "log.hpp"
#include "persist_context.hpp"
#include "persist_manager.hpp"
#include "persist_var.hpp"
#include "play_controller.hpp"
#include "synced_user_choice.hpp"
#include "resources.hpp"
#include "variable.hpp"

#include <cassert>

//TODO: remove LOG_PERSIST, ERR_PERSIST from persist_context.hpp to .cpp files.
#define DBG_PERSIST LOG_STREAM(debug, log_persist)
#define ERR_PERSIST LOG_STREAM(err, log_persist)

struct persist_choice: mp_sync::user_choice {
	const persist_context &ctx;
	std::string var_name;
	int side;
	persist_choice(const persist_context &context,const std::string &name, int side_num)
		: ctx(context)
		, var_name(name)
		, side(side_num) {
	}
	virtual config query_user(int /*side_for*/) const {
		//side can be different from side_for: if side was null-controlled
		//then get_user_choice will use the next non-null-controlled side instead
		config ret;
		ret["side"] = side;
		ret.add_child("variables",ctx.get_var(var_name));
		return ret;
	}
	virtual config random_choice(int /*side_for*/) const {
		return config();
	}

	virtual std::string description() const
	{
		return "a global variable";
	}
	virtual bool is_visible() const { return false; }
};

static void get_global_variable(persist_context &ctx, const vconfig &pcfg)
{
	std::string global = pcfg["from_global"];
	std::string local = pcfg["to_local"];
	config::attribute_value pcfg_side = pcfg["side"];
	const int side = pcfg_side.to_int(resources::controller->current_side());
	persist_choice choice(ctx, global, side);
	config cfg = mp_sync::get_user_choice("global_variable",choice,side).child("variables");
	try
	{
		if (cfg) {
			size_t arrsize = cfg.child_count(global);
			if (arrsize == 0) {
				resources::gamedata->set_variable(local,cfg[global]);
			} else {
				resources::gamedata->clear_variable(local);
				for (size_t i = 0; i < arrsize; i++)
					resources::gamedata->add_variable_cfg(local,cfg.child(global,i));
			}
		} else {
			resources::gamedata->set_variable(local,"");
		}
	}
	catch(const invalid_variablename_exception&)
	{
		ERR_PERSIST << "cannot store global variable into invalid variablename " << local << std::endl;
	}
}

static void clear_global_variable(persist_context &ctx, const vconfig &pcfg)
{
	std::string global = pcfg["global"];
	ctx.clear_var(global, pcfg["immediate"].to_bool());
}

static void set_global_variable(persist_context &ctx, const vconfig &pcfg)
{
	if (pcfg["from_local"].empty()) {
		clear_global_variable(ctx, pcfg);
	} else {
		std::string global = pcfg["to_global"];
		std::string local = pcfg["from_local"];
		config val;
		const config &vars = resources::gamedata->get_variables();
		size_t arraylen = vars.child_count(local);
		if (arraylen == 0) {
			try
			{
				val = pack_scalar(global,resources::gamedata->get_variable(local));
			}
			catch(const invalid_variablename_exception&)
			{
				val = config();
			}
		} else {
			for (size_t i = 0; i < arraylen; i++)
				val.add_child(global,vars.child(local,i));
		}
		ctx.set_var(global, val, pcfg["immediate"].to_bool());
	}
}
void verify_and_get_global_variable(const vconfig &pcfg)
{
	bool valid = true;
	if (!pcfg.has_attribute("from_global")) {
		ERR_PERSIST << "[get_global_variable] missing required attribute \"from_global\"";
		valid = false;
	}
	if (!pcfg.has_attribute("to_local")) {
		ERR_PERSIST << "[get_global_variable] missing required attribute \"to_local\"";
		valid = false;
	}
	// TODO: allow for global namespace.
	if (!pcfg.has_attribute("namespace")) {
		ERR_PERSIST << "[get_global_variable] missing attribute \"namespace\"";
		valid = false;
	}
	if (resources::controller->is_networked_mp()) {
			DBG_PERSIST << "verify_and_get_global_variable with from_global=" << pcfg["from_global"] << " from side " << pcfg["side"] << "\n";
			config::attribute_value pcfg_side = pcfg["side"];
			int side = (pcfg_side.str() == "global" || pcfg_side.empty()) ? resources::controller->current_side() : pcfg_side.to_int();
			if (unsigned (side - 1) >= resources::gameboard->teams().size()) {
				ERR_PERSIST << "[get_global_variable] attribute \"side\" specifies invalid side number." << "\n";
				valid = false;
			}
			DBG_PERSIST <<  "end verify_and_get_global_variable with from_global=" << pcfg["from_global"] << " from side " << pcfg["side"] << "\n";
	}
	if (valid)
	{
		persist_context &ctx = resources::persist->get_context((pcfg["namespace"]));
		if (ctx.valid()) {
			get_global_variable(ctx,pcfg);
		} else {
			LOG_PERSIST << "Error: [get_global_variable] attribute \"namespace\" is not valid.";
		}
	}
}
void verify_and_set_global_variable(const vconfig &pcfg)
{
	bool valid = true;
	if (!pcfg.has_attribute("to_global")) {
		ERR_PERSIST << "[set_global_variable] missing required attribute \"to_global\"";
		valid = false;
	}
	if (!pcfg.has_attribute("from_local")) {
		LOG_PERSIST << "Warning: [set_global_variable] missing attribute \"from_local\", global variable will be cleared";
	}
	// TODO: allow for global namespace.
	if (!pcfg.has_attribute("namespace")) {
		ERR_PERSIST << "[set_global_variable] missing attribute \"namespace\" and no global namespace provided.";
		valid = false;
	}
	if (resources::controller->is_networked_mp()) {
		config::attribute_value pcfg_side = pcfg["side"];
		int side = pcfg_side;
		//Check side matching only if the side is not "global" or empty.
		if (pcfg_side.str() != "global" && !pcfg_side.empty()) {
			//Ensure that the side is valid.
			if (unsigned(side-1) > resources::gameboard->teams().size()) {
				ERR_PERSIST << "[set_global_variable] attribute \"side\" specifies invalid side number.";
				valid = false;
			} else if (resources::gameboard->teams()[side - 1].is_empty()) {
				LOG_PERSIST << "[set_global_variable] attribute \"side\" specifies a null-controlled side number.";
				valid = false;
			} else {
				//Set the variable only if it is meant for a side we control
				valid = resources::gameboard->teams()[side - 1].is_local();
			}
		}
	}
	if (valid)
	{
		persist_context &ctx = resources::persist->get_context((pcfg["namespace"]));
		if (ctx.valid()) {
			set_global_variable(ctx,pcfg);
		} else {
			LOG_PERSIST << "Error: [set_global_variable] attribute \"namespace\" is not valid.";
		}
	}
}
void verify_and_clear_global_variable(const vconfig &pcfg)
{
	bool valid = true;
	if (!pcfg.has_attribute("global")) {
		ERR_PERSIST << "[clear_global_variable] missing required attribute \"from_global\"";
		valid = false;
	}
	if (!pcfg.has_attribute("namespace")) {
		ERR_PERSIST << "[clear_global_variable] missing attribute \"namespace\" and no global namespace provided.";
		valid = false;
	}
	if (resources::controller->is_networked_mp()) {
		config::attribute_value pcfg_side = pcfg["side"];
		const int side = pcfg_side.to_int();
		//Check side matching only if the side is not "global" or empty.
		if (pcfg_side.str() != "global" && !pcfg_side.empty()) {
			//Ensure that the side is valid.
			if (unsigned(side-1) > resources::gameboard->teams().size()) {
				ERR_PERSIST << "[clear_global_variable] attribute \"side\" specifies invalid side number.";
				valid = false;
			} else if (resources::gameboard->teams()[side - 1].is_empty()) {
				LOG_PERSIST << "[clear_global_variable] attribute \"side\" specifies a null-controlled side number.";
				valid = false;
			} else {
				//Clear the variable only if it is meant for a side we control
				valid = resources::gameboard->teams()[side - 1].is_local();
			}
		}
	}
	if (valid)
	{
		persist_context &ctx = resources::persist->get_context((pcfg["namespace"]));
		if (ctx.valid()) {
			clear_global_variable(ctx,pcfg);
		} else {
			LOG_PERSIST << "Error: [clear_global_variable] attribute \"namespace\" is not valid.";
		}
	}
}
