/* $Id$ */
/*
   Copyright (C) 2003 - 2010 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "gamestatus.hpp"
#include "log.hpp"
#include "persist_context.hpp"
#include "persist_var.hpp"
#include "resources.hpp"


static void get_global_variable(persist_context &ctx, const vconfig &pcfg)
{
	std::string global = pcfg["from_global"];
	std::string local = pcfg["to_local"];
	config cfg = ctx.get_var(global);
	if (cfg) {
		if (cfg.has_attribute(global)) {
			resources::state_of_game->set_variable(local,cfg[global]);
		} else {
			resources::state_of_game->clear_variable(local);
			resources::state_of_game->add_variable_cfg(local,cfg.child(global));
		}
	} else {
		resources::state_of_game->set_variable(local,"");
	}
}

static void clear_global_variable(persist_context &ctx, const vconfig &pcfg)
{
	std::string global = pcfg["global"];
	ctx.clear_var(global);
}

static void set_global_variable(persist_context &ctx, const vconfig &pcfg)
{
	if (pcfg["from_local"].empty()) {
		clear_global_variable(ctx, pcfg);
	} else {
		std::string global = pcfg["to_global"];
		std::string local = pcfg["from_local"];
		config val;
		val.add_child(global,resources::state_of_game->get_variable_cfg(local));
		if (val.child(global).empty()) {
			val = pack_scalar(global,resources::state_of_game->get_variable(local));
		}
		ctx.set_var(global,val);
	}
}
void verify_and_get_global_variable(const vconfig &pcfg)
{
	bool valid = true;
	if (!pcfg.has_attribute("from_global")) {
		LOG_SAVE << "Error: [get_global_variable] missing required attribute \"from_global\"";
		valid = false;
	}
	if (!pcfg.has_attribute("to_local")) {
		LOG_SAVE << "Error: [get_global_variable] missing required attribute \"to_local\"";
		valid = false;
	}
	// TODO: allow for global namespace.
	if (!pcfg.has_attribute("namespace")) {
		LOG_SAVE << "Error: [get_global_variable] missing attribute \"namespace\" and no global namespace provided.";
		valid = false;
	}
	// TODO: determine single or multiplayer and check for side=, depending.
	if (valid)
	{
		persist_context ctx(pcfg["namespace"]);
		if (ctx.valid()) {
			get_global_variable(ctx,pcfg);
		} else {
			LOG_SAVE << "Error: [get_global_variable] attribute \"namespace\" is not valid.";
		}
	}
}
void verify_and_set_global_variable(const vconfig &pcfg)
{
	bool valid = true;
	if (!pcfg.has_attribute("to_global")) {
		LOG_SAVE << "Error: [set_global_variable] missing required attribute \"from_global\"";
		valid = false;
	}
	if (!pcfg.has_attribute("from_local")) {
		LOG_SAVE << "Warning: [set_global_variable] missing attribute \"to_local\", global variable will be cleared";
	}
	// TODO: allow for global namespace.
	if (!pcfg.has_attribute("namespace")) {
		LOG_SAVE << "Error: [set_global_variable] missing attribute \"namespace\" and no global namespace provided.";
		valid = false;
	}
	// TODO: determine single or multiplayer and check for side=, depending.
	if (valid)
	{
		persist_context ctx(pcfg["namespace"]);
		if (ctx.valid()) {
			set_global_variable(ctx,pcfg);
		} else {
			LOG_SAVE << "Error: [set_global_variable] attribute \"namespace\" is not valid.";
		}
	}
}
void verify_and_clear_global_variable(const vconfig &pcfg)
{
	bool valid = true;
	if (!pcfg.has_attribute("global")) {
		LOG_SAVE << "Error: [clear_global_variable] missing required attribute \"from_global\"";
		valid = false;
	}
	if (!pcfg.has_attribute("namespace")) {
		LOG_SAVE << "Error: [clear_global_variable] missing attribute \"namespace\" and no global namespace provided.";
		valid = false;
	}
	// TODO: determine single or multiplayer and check for side=, depending.
	if (valid)
	{
		persist_context ctx(pcfg["namespace"]);
		if (ctx.valid()) {
			clear_global_variable(ctx,pcfg);
		} else {
			LOG_SAVE << "Error: [clear_global_variable] attribute \"namespace\" is not valid.";
		}
	}
}
