/* $Id: value_translator.hpp 38379 2009 - 2010-09-03 23:23:37Z crab $ */
/*
   Copyright (C) 2009 - 2010 by Yurii Chernyi <terraninfo@terraninfo.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file ai/composite/value_translator.hpp
 */


#include "../../global.hpp"

#include "component.hpp"
#include "engine.hpp"
#include "stage.hpp"

#include "../contexts.hpp"
#include "../default/contexts.hpp"
#include "../game_info.hpp"
#include "../manager.hpp"
#include "../../foreach.hpp"
#include "../../log.hpp"
#include "../../terrain_filter.hpp"

#include <map>
#include <stack>
#include <vector>
#include <deque>
#include <iterator>
#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <boost/pointer_cast.hpp>


namespace ai {

template<typename T>
class config_value_translator {
public:

	static void cfg_to_value(const config &cfg, T &value)
	{
		try {
			value = boost::lexical_cast<T>(cfg["value"]);
		} catch (boost::bad_lexical_cast e) {
			//@todo: 1.7.11 handle error, at least log it
		}
	}

	static void value_to_cfg(const T &value, config &cfg)
	{
		try {
			cfg["value"] = boost::lexical_cast<std::string>(value);
		} catch (boost::bad_lexical_cast e) {
			//@todo: 1.7.11 handle error, at least log it
		}
	}

	static config value_to_cfg(const T &value)
	{
		config cfg;
		value_to_cfg(value,cfg);
		return cfg;
	}

	static T cfg_to_value(const config &cfg)
	{
		T value;
		cfg_to_value(cfg,value);
		return value;
	}
};


template<>
class config_value_translator<bool> {
public:

	static void cfg_to_value(const config &cfg, bool &value)
	{
		value = utils::string_bool(cfg["value"]);
	}

	static void value_to_cfg(const bool &value, config &cfg)
	{
		cfg["value"] = value ? "yes" : "no";
	}

	static config value_to_cfg(const bool &value)
	{
		config cfg;
		value_to_cfg(value,cfg);
		return cfg;
	}

	static bool cfg_to_value(const config &cfg)
	{
		bool value;
		cfg_to_value(cfg,value);
		return value;
	}
};


template<>
class config_value_translator<int> {
public:

	static void cfg_to_value(const config &cfg, int &value)
	{
		value = atoi(cfg["value"].c_str());
	}

	static void value_to_cfg(const int &value, config &cfg)
	{
		cfg["value"] = boost::lexical_cast<std::string>(value);
	}

	static config value_to_cfg(const int &value)
	{
		config cfg;
		value_to_cfg(value,cfg);
		return cfg;
	}

	static int cfg_to_value(const config &cfg)
	{
		int value;
		cfg_to_value(cfg,value);
		return value;
	}
};


template<>
class config_value_translator< std::vector<std::string> > {
public:

	static void cfg_to_value(const config &cfg, std::vector<std::string> &value)
	{
		value = utils::split(cfg["value"]);
	}

	static void value_to_cfg(const std::vector<std::string> &value, config &cfg)
	{
		std::stringstream buf;
		for(std::vector<std::string>::const_iterator p = value.begin(); p != value.end(); ++p) {
			if (p != value.begin()) {
				buf << ",";
			}
			buf << *p;
		}
		cfg["value"] = buf.str();
	}

	static config value_to_cfg(const std::vector<std::string> &value)
	{
		config cfg;
		value_to_cfg(value,cfg);
		return cfg;
	}

	static std::vector<std::string> cfg_to_value(const config &cfg)
	{
		std::vector<std::string> value;
		cfg_to_value(cfg,value);
		return value;
	}
};

template<>
class config_value_translator<config> {
public:

	static void cfg_to_value(const config &cfg, config &value)
	{
		if (cfg.child("value")) {
			value = cfg.child("value");
		} else {
			value = config();
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
		config value;
		cfg_to_value(cfg,value);
		return value;
	}
};

template<>
class config_value_translator<ministage> {
public:

	static void cfg_to_value(const config &cfg, ministage &value)
	{
		value = ministage(cfg.child_or_empty("value"));
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

	static ministage cfg_to_value(const config &cfg)
	{
		config c;
		ministage value(c);
		cfg_to_value(cfg,value);
		return value;
	}
};

template<>
class config_value_translator<terrain_filter> {
public:

	static void cfg_to_value(const config &cfg, terrain_filter &value)
	{
		if (cfg.child("value")) {
			value = terrain_filter(vconfig(cfg.child("value")),manager::get_ai_info().units);
		} else {
			static config c;
			if (!c.child("not")) {
				c.add_child("not");
			}
			value = terrain_filter(vconfig(c),manager::get_ai_info().units);
		}
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

	static terrain_filter cfg_to_value(const config &cfg)
	{
		if (cfg.child("value")) {
			return terrain_filter(vconfig(cfg.child("value")),manager::get_ai_info().units);
		}
		static config c;
		if (!c.child("not")) {
			c.add_child("not");
		}
		return terrain_filter(vconfig(c),manager::get_ai_info().units);
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
		T value;
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
		static config c;
		if (!c.child("not")) {
			c.add_child("not");
		}

		terrain_filter value(vconfig(c),manager::get_ai_info().units);
		variant_to_value(var,value);
		return value;
	}
};
}  //end of namespace ai
