/*
   Copyright (C) 2009 - 2016 by Yurii Chernyi <terraninfo@terraninfo.net>
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

#ifndef VALUE_TRANSLATOR_HPP_INCLUDED
#define VALUE_TRANSLATOR_HPP_INCLUDED

#include "engine.hpp"
#include "stage.hpp"

#include "../manager.hpp"
#include "../../terrain_filter.hpp"
#include "../../util.hpp"
#include "../../serialization/string_utils.hpp"
#include "../../resources.hpp"
#include "../lua/unit_advancements_aspect.hpp"

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
		cfg["value"] = str_cast(value);
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
class config_value_translator< std::vector<std::string> > {
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
class config_value_translator<ministage> {
public:

	static ministage cfg_to_value(const config &cfg)
	{
		return ministage(cfg.child_or_empty("value"));
	}

	static void cfg_to_value(const config &cfg, ministage &value)
	{
		value = cfg_to_value(cfg);
	}

	static void value_to_cfg(const ministage &value, config &cfg)
	{
		cfg.add_child("value",value.to_config());
	}

	static config value_to_cfg(const ministage &value)
	{
		config cfg;
		value_to_cfg(value,cfg);
		return cfg;
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

	static void variant_to_value(const variant &/*var*/, T &/*value*/)
	{
	        assert(false);//not implemented
	}

	static void value_to_variant(const T &/*value*/, variant &/*var*/)
	{
		assert(false);//not implemented
	}

	static variant value_to_variant(const T &value)
	{
		variant var;
		value_to_variant(value,var);
		return var;
	}

	static T variant_to_value(const variant &var)
	{
		T value = T();
		variant_to_value(var,value);
		return value;
	}
};

template<>
class variant_value_translator<ministage> {
public:

	static void variant_to_value(const variant &/*var*/, ministage &/*value*/)
	{
	        assert(false);//not implemented
	}

	static void value_to_variant(const ministage &/*value*/, variant &/*var*/)
	{
		assert(false);//not implemented
	}

	static variant value_to_variant(const ministage &/*value*/)
	{
		assert(false);
		return variant();
	}

	static ministage variant_to_value(const variant &/*var*/)
	{
		assert(false);
		config cfg;
		return ministage(cfg);
	}
};

template<>
class variant_value_translator<int> {
public:

	static void variant_to_value(const variant &var, int &value)
	{
	        value = var.as_int();
	}

	static void value_to_variant(const int &value, variant &var)
	{
		var = variant(value);
	}

	static variant value_to_variant(const int &value)
	{
		variant var;
		value_to_variant(value,var);
		return var;
	}

	static int variant_to_value(const variant &var)
	{
		int value;
		variant_to_value(var,value);
		return value;
	}
};


template<>
class variant_value_translator<bool> {
public:

	static void variant_to_value(const variant &var, bool &value)
	{
	        value = var.as_bool();
	}

	static void value_to_variant(const bool &value, variant &var)
	{
		var = variant(value);
	}

	static variant value_to_variant(const bool &value)
	{
		variant var;
		value_to_variant(value,var);
		return var;
	}

	static bool variant_to_value(const variant &var)
	{
		bool value;
		variant_to_value(var,value);
		return value;
	}
};



template<>
class variant_value_translator<std::string> {
public:

	static void variant_to_value(const variant &var, std::string &value)
	{
	        value = var.as_string();
	}

	static void value_to_variant(const std::string &value, variant &var)
	{
		var = variant(value);
	}

	static variant value_to_variant(const std::string &value)
	{
		variant var;
		value_to_variant(value,var);
		return var;
	}

	static std::string variant_to_value(const variant &var)
	{
		std::string value;
		variant_to_value(var,value);
		return value;
	}
};



template<>
class variant_value_translator<attacks_vector> {
public:

	static void variant_to_value(const variant &/*var*/, attacks_vector &/*value*/)
	{
		assert(false);//not implemented
	}

	static void value_to_variant(const attacks_vector &value, variant &var)
	{
                std::vector<variant> vars;
                for(attacks_vector::const_iterator i = value.begin(); i != value.end(); ++i) {
                        vars.push_back(variant(new attack_analysis(*i)));
                }
		var = variant(&vars);
	}

	static variant value_to_variant(const attacks_vector &value)
	{
		variant var;
		value_to_variant(value,var);
		return var;
	}

	static attacks_vector variant_to_value(const variant &var)
	{
		attacks_vector value;
		variant_to_value(var,value);
		return value;
	}
};


template<>
class variant_value_translator<terrain_filter> {
public:

	static void variant_to_value(const variant &/*var*/, terrain_filter &/*value*/)
	{
	        assert(false);//not implemented
	}

	static void value_to_variant(const terrain_filter &/*value*/, variant &/*var*/)
	{
		assert(false);//not implemented
	}

	static variant value_to_variant(const terrain_filter &value)
	{
		variant var;
		value_to_variant(value,var);
		return var;
	}

	static terrain_filter variant_to_value(const variant &var)
	{
		static config c("not");
		terrain_filter value(vconfig(c),resources::filter_con);
		variant_to_value(var,value);
		return value;
	}
};
}  //end of namespace ai

#endif
