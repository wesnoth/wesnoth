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
#include "units/unit.hpp"

class team;
class terrain_type;

namespace game_logic
{

class terrain_callable : public formula_callable
{
public:
	terrain_callable(const terrain_type& t, const map_location& loc) : loc_(loc), t_(t)
	{
		type_ = TERRAIN_C;
	}

	variant get_value(const std::string& key) const override;
	void get_inputs(formula_input_vector* inputs) const override;

	int do_compare(const formula_callable* callable) const override;

private:
	const map_location loc_;
	const terrain_type& t_;
};

class gamemap_callable : public formula_callable
{
public:
	explicit gamemap_callable(const gamemap& g) : gamemap_(g)
	{}

	void get_inputs(formula_input_vector* inputs) const override;
	variant get_value(const std::string& key) const override;

	const gamemap& get_gamemap() const { return gamemap_; }

private:
	const gamemap& gamemap_;
};

class location_callable : public formula_callable
{
public:
	explicit location_callable(const map_location& loc) : loc_(loc)
	{
		type_ = LOCATION_C;
	}

	void serialize_to_string(std::string& str) const override;

	const map_location& loc() const { return loc_; }

private:
	map_location loc_;

	variant get_value(const std::string& key) const override;

	void get_inputs(formula_input_vector* inputs) const override;
	int do_compare(const formula_callable* callable) const override;
};

class attack_type_callable : public formula_callable
{
public:
	explicit attack_type_callable(const attack_type& attack) : att_(attack)
	{
		type_ = ATTACK_TYPE_C;
	}

	variant get_value(const std::string& key) const override;
	void get_inputs(formula_input_vector* inputs) const override;

	int do_compare(const formula_callable* callable) const override;

	const attack_type& get_attack_type() const { return att_; }

private:
	const attack_type att_;
};

class unit_callable : public formula_callable
{
public:
	unit_callable(const map_location& loc, const unit& u) : loc_(loc), u_(u)
	{
		type_ = UNIT_C;
	}

	explicit unit_callable(const unit &u) : loc_(u.get_location()), u_(u)
	{
		type_ = UNIT_C;
	}

	variant get_value(const std::string& key) const override;
	void get_inputs(formula_input_vector* inputs) const override;

	int do_compare(const formula_callable* callable) const override;

	const unit& get_unit() const { return u_; }
	const map_location& get_location() const { return loc_; }

private:
	const map_location& loc_;
	const unit& u_;
};

class unit_type_callable : public formula_callable
{
public:
	explicit unit_type_callable(const unit_type& u) : u_(u)
	{
		type_ = UNIT_TYPE_C;
	}

	variant get_value(const std::string& key) const override;
	void get_inputs(formula_input_vector* inputs) const override;

	int do_compare(const formula_callable* callable) const override;

	const unit_type& get_unit_type() const { return u_; }

private:
	const unit_type& u_;
};

class config_callable : public formula_callable
{
public:
	explicit config_callable(const config& c) : cfg_(c) {}

	variant get_value(const std::string& key) const override;
	void get_inputs(formula_input_vector* inputs) const override;
	int do_compare(const formula_callable* callable) const override;

	const config& get_config() const { return cfg_; }

private:
	const config& cfg_;
};

class team_callable : public formula_callable
{
public:
	explicit team_callable(const team& t) : team_(t)
	{}

	void get_inputs(formula_input_vector* inputs) const override;
	variant get_value(const std::string& key) const override;

	const team& get_team() const { return team_; }

private:
	const team& team_;
};

} // namespace game_logic

#endif
