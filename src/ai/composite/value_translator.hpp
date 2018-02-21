/*
   Copyright (C) 2009 - 2018 by Yurii Chernyi <terraninfo@terraninfo.net>
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
 */

#pragma once

#include "ai/composite/engine.hpp"
#include "ai/composite/stage.hpp"

#include "ai/manager.hpp"
#include "terrain/filter.hpp"
#include "lexical_cast.hpp"
#include "serialization/string_utils.hpp"
#include "resources.hpp"
#include "ai/lua/aspect_advancements.hpp"

namespace ai {

template<typename T>
class config_value_translator {
public:

	static T cfg_to_value(const config &cfg)
	{
		return lexical_cast_default<T>(cfg["value"]);
	}

	static void cfg_to_value(const config &cfg, T &value)
	{
		value = cfg_to_value(cfg);
	}

	static void value_to_cfg(const T &value, config &cfg)
	{
		cfg["value"] = lexical_cast<std::string>(value);
	}

	static config value_to_cfg(const T &value)
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
		if (const config &v = cfg.child("value")) {
			value = v;
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
		if (const config &v = cfg.child("value")) {
			return terrain_filter(vconfig(v), resources::filter_con);
		}
		static config c("not");
		return terrain_filter(vconfig(c),resources::filter_con);
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



// variant value translator

template<typename T>
class variant_value_translator {
public:

	static void variant_to_value(const wfl::variant &/*var*/, T &/*value*/)
	{
	        assert(false);//not implemented
	}

	static void value_to_variant(const T &/*value*/, wfl::variant &/*var*/)
	{
		assert(false);//not implemented
	}

	static wfl::variant value_to_variant(const T &value)
	{
		wfl::variant var;
		value_to_variant(value,var);
		return var;
	}

	static T variant_to_value(const wfl::variant &var)
	{
		T value = T();
		variant_to_value(var,value);
		return value;
	}
};

template<>
class variant_value_translator<int> {
public:

	static void variant_to_value(const wfl::variant &var, int &value)
	{
	        value = var.as_int();
	}

	static void value_to_variant(const int &value, wfl::variant &var)
	{
		var = wfl::variant(value);
	}

	static wfl::variant value_to_variant(const int &value)
	{
		wfl::variant var;
		value_to_variant(value,var);
		return var;
	}

	static int variant_to_value(const wfl::variant &var)
	{
		int value;
		variant_to_value(var,value);
		return value;
	}
};


template<>
class variant_value_translator<bool> {
public:

	static void variant_to_value(const wfl::variant &var, bool &value)
	{
	        value = var.as_bool();
	}

	static void value_to_variant(const bool &value, wfl::variant &var)
	{
		var = wfl::variant(value);
	}

	static wfl::variant value_to_variant(const bool &value)
	{
		wfl::variant var;
		value_to_variant(value,var);
		return var;
	}

	static bool variant_to_value(const wfl::variant &var)
	{
		bool value;
		variant_to_value(var,value);
		return value;
	}
};



template<>
class variant_value_translator<std::string> {
public:

	static void variant_to_value(const wfl::variant &var, std::string &value)
	{
	        value = var.as_string();
	}

	static void value_to_variant(const std::string &value, wfl::variant &var)
	{
		var = wfl::variant(value);
	}

	static wfl::variant value_to_variant(const std::string &value)
	{
		wfl::variant var;
		value_to_variant(value,var);
		return var;
	}

	static std::string variant_to_value(const wfl::variant &var)
	{
		std::string value;
		variant_to_value(var,value);
		return value;
	}
};



template<>
class variant_value_translator<attacks_vector> {
public:

	static void variant_to_value(const wfl::variant &/*var*/, attacks_vector &/*value*/)
	{
		assert(false);//not implemented
	}

	static void value_to_variant(const attacks_vector &value, wfl::variant &var)
	{
                std::vector<wfl::variant> vars;
                for(attacks_vector::const_iterator i = value.begin(); i != value.end(); ++i) {
                        vars.emplace_back(std::make_shared<attack_analysis>(*i));
                }
		var = wfl::variant(vars);
	}

	static wfl::variant value_to_variant(const attacks_vector &value)
	{
		wfl::variant var;
		value_to_variant(value,var);
		return var;
	}

	static attacks_vector variant_to_value(const wfl::variant &var)
	{
		attacks_vector value;
		variant_to_value(var,value);
		return value;
	}
};


template<>
class variant_value_translator<terrain_filter> {
public:

	static void variant_to_value(const wfl::variant &/*var*/, terrain_filter &/*value*/)
	{
	        assert(false);//not implemented
	}

	static void value_to_variant(const terrain_filter &/*value*/, wfl::variant &/*var*/)
	{
		assert(false);//not implemented
	}

	static wfl::variant value_to_variant(const terrain_filter &value)
	{
		wfl::variant var;
		value_to_variant(value,var);
		return var;
	}

	static terrain_filter variant_to_value(const wfl::variant &var)
	{
		static config c("not");
		terrain_filter value(vconfig(c),resources::filter_con);
		variant_to_value(var,value);
		return value;
	}
};
}  //end of namespace ai
