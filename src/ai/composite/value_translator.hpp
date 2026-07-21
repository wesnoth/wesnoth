/*
	Copyright (C) 2009 - 2025
	by Yurii Chernyi <terraninfo@terraninfo.net>
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
 */

#pragma once

#include "ai/composite/engine.hpp"
#include "ai/composite/stage.hpp"
#include "ai/lua/aspect_advancements.hpp"
#include "ai/manager.hpp"
#include "lexical_cast.hpp"
#include "resources.hpp"
#include "serialization/string_utils.hpp"
#include "terrain/filter.hpp"
#include "utils/variant.hpp"

namespace ai {

template<typename T>
class config_value_translator {
public:

	static T cfg_to_value(const config &cfg)
	{
		return cfg["value"].to(T{});
	}

	static void cfg_to_value(const config &cfg, T &value)
	{
		value = cfg_to_value(cfg);
	}

	static void value_to_cfg(const T &value, config &cfg)
	{
		cfg["value"] = value;
	}

	static config value_to_cfg(const T &value)
	{
		config cfg;
		value_to_cfg(value,cfg);
		return cfg;
	}
};

template<>
class config_value_translator<std::string> {
public:

	static std::string cfg_to_value(const config &cfg)
	{
		return cfg["value"].str();
	}

	static void cfg_to_value(const config &cfg, std::string &value)
	{
		value = cfg_to_value(cfg);
	}

	static void value_to_cfg(const std::string &value, config &cfg)
	{
		cfg["value"] = value;
	}

	static config value_to_cfg(const std::string &value)
	{
		config cfg;
		value_to_cfg(value,cfg);
		return cfg;
	}

};

template<>
class config_value_translator<bool> {
public:

	static bool cfg_to_value(const config &cfg)
	{
		return cfg["value"].to_bool();
	}

	static void cfg_to_value(const config &cfg, bool &value)
	{
		value = cfg_to_value(cfg);
	}

	static void value_to_cfg(const bool &value, config &cfg)
	{
		cfg["value"] = value;
	}

	static config value_to_cfg(const bool &value)
	{
		config cfg;
		value_to_cfg(value,cfg);
		return cfg;
	}
};

class leader_aspects_visitor
#ifdef USING_BOOST_VARIANT
	: public boost::static_visitor<std::string>
#endif
{
public:
	std::string operator()(const bool b) const {
		if (b) {
			return "yes";
		} else {
			return "no";
		}
	}
	std::string operator()(const std::vector<std::string> s) const { return utils::join(s); }
};

template<>
class config_value_translator<utils::variant<bool, std::vector<std::string>>> {
public:

	static utils::variant<bool, std::vector<std::string>> cfg_to_value(const config &cfg)
	{
		if (cfg["value"].to_bool(true) == cfg["value"].to_bool(false)) {
			return cfg["value"].to_bool();
		}
		return utils::split(cfg["value"]);
	}

	static void cfg_to_value(const config &cfg, utils::variant<bool, std::vector<std::string>> &value)
	{
		value = cfg_to_value(cfg);
	}

	static void value_to_cfg(const utils::variant<bool, std::vector<std::string>> &value, config &cfg)
	{
		cfg["value"] = utils::visit(leader_aspects_visitor(), value);
	}

	static config value_to_cfg(const utils::variant<bool, std::vector<std::string>> &value)
	{
		config cfg;
		value_to_cfg(value,cfg);
		return cfg;
	}
};

template<>
class config_value_translator< std::vector<std::string>> {
public:

	static std::vector<std::string> cfg_to_value(const config &cfg)
	{
		return utils::split(cfg["value"]);
	}

	static void cfg_to_value(const config &cfg, std::vector<std::string> &value)
	{
		value = cfg_to_value(cfg);
	}

	static void value_to_cfg(const std::vector<std::string> &value, config &cfg)
	{
		cfg["value"] = utils::join(value);
	}

	static config value_to_cfg(const std::vector<std::string> &value)
	{
		config cfg;
		value_to_cfg(value,cfg);
		return cfg;
	}
};

template<>
class config_value_translator<config> {
public:

	static void cfg_to_value(const config &cfg, config &value)
	{
		if (auto v = cfg.optional_child("value")) {
			value = *v;
		} else {
			value.clear();
		}
	}

	static void value_to_cfg(const config &value, config &cfg)
	{
		cfg.add_child("value",value);
	}

	static config value_to_cfg(const config &value)
	{
		config cfg;
		value_to_cfg(value,cfg);
		return cfg;
	}

	static config cfg_to_value(const config &cfg)
	{
		return cfg.child_or_empty("value");
	}
};

template<>
class config_value_translator<terrain_filter> {
public:

	static terrain_filter cfg_to_value(const config &cfg)
	{
		if (auto v = cfg.optional_child("value")) {
			return terrain_filter(vconfig(*v), resources::filter_con, false);
		}
		static config c("not");
		return terrain_filter(vconfig(c),resources::filter_con, false);
	}

	static void cfg_to_value(const config &cfg, terrain_filter &value)
	{
		value = cfg_to_value(cfg);
	}

	static void value_to_cfg(const terrain_filter &value, config &cfg)
	{
		cfg.add_child("value",value.to_config());
	}

	static config value_to_cfg(const terrain_filter &value)
	{
		config cfg;
		value_to_cfg(value,cfg);
		return cfg;
	}
};

template<>
class config_value_translator<unit_advancements_aspect> {
public:

	static unit_advancements_aspect cfg_to_value(const config &cfg)
	{
		return unit_advancements_aspect(cfg["value"]);
	}

	static void cfg_to_value(const config &cfg, unit_advancements_aspect &value)
	{
		value = cfg_to_value(cfg);
	}

	static void value_to_cfg(const unit_advancements_aspect &value, config &cfg)
	{
		cfg["value"] = value.get_value();

	}

	static config value_to_cfg(const unit_advancements_aspect &value)
	{
		config cfg;
		value_to_cfg(value,cfg);
		return cfg;
	}
};

}  //end of namespace ai
