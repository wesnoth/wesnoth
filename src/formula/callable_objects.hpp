/*
   Copyright (C) 2014 - 2017 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/


#ifndef CALLABLE_OBJECTS_HPP_INCLUDED
#define CALLABLE_OBJECTS_HPP_INCLUDED

#include "formula/callable.hpp"
#include "map/map.hpp"
#include "team.hpp"
#include "units/unit.hpp"

template <typename T, typename K> variant convert_map( const std::map<T,K>& map );

template <typename T> variant convert_vector( const std::vector<T>& input_vector );

class terrain_callable : public game_logic::formula_callable {
public:
	typedef map_location location;
	terrain_callable(const terrain_type& t, const location& loc)
	  : loc_(loc), t_(t)
	{
		type_ = TERRAIN_C;
	}

	variant get_value(const std::string& key) const;
	void get_inputs(std::vector<game_logic::formula_input>* inputs) const;

	int do_compare(const formula_callable* callable) const;
private:
	const location loc_;
	const terrain_type &t_;
};

class gamemap_callable : public game_logic::formula_callable
{
	const gamemap& object_;
public:
	explicit gamemap_callable(const gamemap& object) : object_(object)
	{}

	const gamemap& get_gamemap() const { return object_; }
	void get_inputs(std::vector<game_logic::formula_input>* inputs) const
	{
		using game_logic::FORMULA_READ_ONLY;
		inputs->push_back(game_logic::formula_input("gamemap", FORMULA_READ_ONLY));
		inputs->push_back(game_logic::formula_input("terrain", FORMULA_READ_ONLY));
		inputs->push_back(game_logic::formula_input("w", FORMULA_READ_ONLY));
		inputs->push_back(game_logic::formula_input("h", FORMULA_READ_ONLY));
	}

	variant get_value(const std::string& key) const {
	if(key == "terrain") {
		int w = object_.w();
		int h = object_.h();
		std::vector<variant> vars;
		for(int i = 0;i < w; i++) {
			for(int j = 0;j < h; j++) {
				const map_location loc(i,j);
				vars.push_back(variant(new terrain_callable(object_.get_terrain_info(loc), loc)));
			}
		}
		return variant(&vars);
	} else
		if(key == "w") {
		return variant(object_.w());
	} else
	if(key == "h") {
		return variant(object_.h());
	} else
		{ return variant(); }
	}
};


class location_callable : public game_logic::formula_callable {
	map_location loc_;

	variant get_value(const std::string& key) const;

	void get_inputs(std::vector<game_logic::formula_input>* inputs) const;
	int do_compare(const game_logic::formula_callable* callable) const;
public:
	explicit location_callable(const map_location& loc) : loc_(loc)
	{
		type_ = LOCATION_C;
	}

	const map_location& loc() const { return loc_; }

	void serialize_to_string(std::string& str) const;
};


class attack_type_callable : public game_logic::formula_callable {
public:
	typedef map_location location;
	attack_type_callable(const attack_type& attack)
	  : att_(attack)
	{
		type_ = ATTACK_TYPE_C;
	}

	const attack_type& get_attack_type() const { return att_; }
	variant get_value(const std::string& key) const;
	void get_inputs(std::vector<game_logic::formula_input>* inputs) const;

	int do_compare(const formula_callable* callable) const;
private:
	const attack_type att_;
};


class unit_callable : public game_logic::formula_callable {
public:
	typedef map_location location;
	unit_callable(const location& loc, const unit& u)
		: loc_(loc), u_(u)
	{
		type_ = UNIT_C;
	}

	unit_callable(const unit &u)
		: loc_(u.get_location()), u_(u)
	{
		type_ = UNIT_C;
	}

	const unit& get_unit() const { return u_; }
	const location& get_location() const { return loc_; }
	variant get_value(const std::string& key) const;
	void get_inputs(std::vector<game_logic::formula_input>* inputs) const;

	int do_compare(const formula_callable* callable) const;
private:
	const location& loc_;
	const unit& u_;
};


class unit_type_callable : public game_logic::formula_callable {
public:
	unit_type_callable(const unit_type& u)
	  : u_(u)
	{
		type_ = UNIT_TYPE_C;
	}

	const unit_type& get_unit_type() const { return u_; }
	variant get_value(const std::string& key) const;
	void get_inputs(std::vector<game_logic::formula_input>* inputs) const;

	int do_compare(const formula_callable* callable) const;
private:
	const unit_type& u_;
};

class config_callable : public game_logic::formula_callable {
public:
	config_callable(const config& c) : cfg_(c) {}
	variant get_value(const std::string& key) const;
	void get_inputs(std::vector<game_logic::formula_input>* inputs) const;
	int do_compare(const formula_callable* callable) const;
	const config& get_config() const {return cfg_;}
private:
	const config& cfg_;
};


class team_callable : public game_logic::formula_callable {
	const team& object_;
public:
	explicit team_callable(const team& object) : object_(object)
	{}

	const team& get_team() const { return object_; }
	void get_inputs(std::vector<game_logic::formula_input>* inputs) const
	{
		using game_logic::FORMULA_READ_ONLY;

		inputs->push_back(game_logic::formula_input("side", FORMULA_READ_ONLY));
		inputs->push_back(game_logic::formula_input("id", FORMULA_READ_ONLY));
		inputs->push_back(game_logic::formula_input("gold", FORMULA_READ_ONLY));
		inputs->push_back(game_logic::formula_input("start_gold", FORMULA_READ_ONLY));
		inputs->push_back(game_logic::formula_input("base_income", FORMULA_READ_ONLY));
		inputs->push_back(game_logic::formula_input("total_income", FORMULA_READ_ONLY));
		inputs->push_back(game_logic::formula_input("village_gold", FORMULA_READ_ONLY));
		inputs->push_back(game_logic::formula_input("village_support", FORMULA_READ_ONLY));
		inputs->push_back(game_logic::formula_input("recall_cost", FORMULA_READ_ONLY));
		inputs->push_back(game_logic::formula_input("name", FORMULA_READ_ONLY));
		inputs->push_back(game_logic::formula_input("is_human", FORMULA_READ_ONLY));
		inputs->push_back(game_logic::formula_input("is_ai", FORMULA_READ_ONLY));
		inputs->push_back(game_logic::formula_input("is_network", FORMULA_READ_ONLY));
		inputs->push_back(game_logic::formula_input("fog", FORMULA_READ_ONLY));
		inputs->push_back(game_logic::formula_input("shroud", FORMULA_READ_ONLY));
		inputs->push_back(game_logic::formula_input("hidden", FORMULA_READ_ONLY));
		inputs->push_back(game_logic::formula_input("flag", FORMULA_READ_ONLY));
		inputs->push_back(game_logic::formula_input("flag_icon", FORMULA_READ_ONLY));
		inputs->push_back(game_logic::formula_input("team_name", FORMULA_READ_ONLY));
		inputs->push_back(game_logic::formula_input("faction", FORMULA_READ_ONLY));
		inputs->push_back(game_logic::formula_input("faction_name", FORMULA_READ_ONLY));
		inputs->push_back(game_logic::formula_input("color", FORMULA_READ_ONLY));
		inputs->push_back(game_logic::formula_input("share_vision", FORMULA_READ_ONLY));
		inputs->push_back(game_logic::formula_input("carryover_bonus", FORMULA_READ_ONLY));
		inputs->push_back(game_logic::formula_input("carryover_percentage", FORMULA_READ_ONLY));
		inputs->push_back(game_logic::formula_input("carryover_add", FORMULA_READ_ONLY));
		inputs->push_back(game_logic::formula_input("recruit", FORMULA_READ_ONLY));
		inputs->push_back(game_logic::formula_input("wml_vars", FORMULA_READ_ONLY));
	}

	variant get_value(const std::string& key) const {
		if(key == "side") {
			return variant(object_.side());
		} else
		if(key == "id") {
			return variant(object_.save_id());
		} else
		if(key == "save_id") {
			return variant(object_.save_id());
		} else
		if(key == "gold") {
			return variant(object_.gold());
		} else
		if(key == "start_gold") {
			return variant(object_.start_gold());
		} else
		if(key == "base_income") {
			return variant(object_.base_income());
		} else
		if(key == "total_income") {
			return variant(object_.total_income());
		} else
		if(key == "village_gold") {
			return variant(object_.village_gold());
		} else
		if(key == "village_support") {
			return variant(object_.village_support());
		} else
		if(key == "recall_cost") {
			return variant(object_.recall_cost());
		} else
		if(key == "is_human") {
			return variant(object_.is_local_human());
		} else
		if(key == "is_ai") {
			return variant(object_.is_local_ai());
		} else
		if(key == "is_network") {
			return variant(object_.is_network());
		} else
		if(key == "fog") {
			return variant(object_.uses_fog());
		} else
		if(key == "shroud") {
			return variant(object_.uses_shroud());
		} else
		if(key == "hidden") {
			return variant(object_.hidden());
		} else
		if(key == "flag") {
			return variant(object_.flag());
		} else
		if(key == "flag_icon") {
			return variant(object_.flag_icon());
		} else
		if(key == "team_name") {
			return variant(object_.team_name());
		} else
		if(key == "color") {
			return variant(object_.color());
		} else
		if(key == "share_vision") {
			return variant(object_.share_vision().to_string());
		} else
		if(key == "carryover_bonus") {
			return variant(object_.carryover_bonus());
		} else
		if(key == "carryover_percentage") {
			return variant(object_.carryover_percentage());
		} else
		if(key == "carryover_add") {
			return variant(object_.carryover_add());
		} else
	if(key == "recruit") {
		const std::set<std::string>& recruits = object_.recruits();
		std::vector<variant> result;
		for(std::set<std::string>::const_iterator it = recruits.begin(); it != recruits.end(); ++it) {
			result.push_back(variant(*it));
		}
		return variant(&result);
	} else if(key == "wml_vars") {
		return variant(new config_callable(object_.variables()));
	} else
		{ return variant(); }
	}
};
#endif
