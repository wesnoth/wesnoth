/*
   Copyright (C) 2014 - 2016 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "formula/callable_objects.hpp"
#include "units/unit.hpp"
#include "units/formula_manager.hpp"

template <typename T, typename K>
variant convert_map( const std::map<T, K>& input_map ) {
	std::map<variant,variant> tmp;

	for(typename std::map< T, K>::const_iterator i = input_map.begin(); i != input_map.end(); ++i) {
			tmp[ variant(i->first) ] = variant( i->second );
	}

	return variant( &tmp );
}


template <typename T>
variant convert_vector( const std::vector<T>& input_vector )
{
	std::vector<variant> tmp;

	for(typename std::vector<T>::const_iterator i = input_vector.begin(); i != input_vector.end(); ++i) {
			tmp.push_back( variant( *i ) );
	}

	return variant( &tmp );
}


variant location_callable::get_value(const std::string& key) const
{
	if(key == "x") {
		return variant(loc_.x+1);
	} else if(key == "y") {
		return variant(loc_.y+1);
	} else {
		return variant();
	}
}

void location_callable::get_inputs(std::vector<game_logic::formula_input>* inputs) const
{
	using game_logic::FORMULA_READ_ONLY;
	inputs->push_back(game_logic::formula_input("x", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("y", FORMULA_READ_ONLY));
}

int location_callable::do_compare(const game_logic::formula_callable* callable) const
{
	const location_callable* loc_callable = dynamic_cast<const location_callable*>(callable);
	if(loc_callable == NULL) {
		return formula_callable::do_compare(callable);
	}

	const map_location& other_loc = loc_callable->loc();
	return loc_.do_compare(other_loc);
}

void location_callable::serialize_to_string(std::string& str) const
{
	std::ostringstream s;
	s << "loc(" << (loc_.x+1) << "," << (loc_.y+1) << ")";
	str += s.str();
}


variant attack_type_callable::get_value(const std::string& key) const
{
	if(key == "id") {
		return variant(att_.id());
	} else if(key == "type") {
		return variant(att_.type());
	} else if(key == "range") {
		return variant(att_.range());
	} else if(key == "damage") {
		return variant(att_.damage());
	} else if(key == "number_of_attacks") {
		return variant(att_.num_attacks());
	} else if(key == "special") {
		std::vector<std::pair<t_string, t_string> > specials = att_.special_tooltips();
		std::vector<variant> res;

		for( size_t i = 0; i != specials.size(); ++i ) {
			res.push_back( variant(specials[i].first.base_str()) );
		}
		return variant(&res);
	}

	return variant();
}

void attack_type_callable::get_inputs(std::vector<game_logic::formula_input>* inputs) const
{
	using game_logic::FORMULA_READ_ONLY;
	inputs->push_back(game_logic::formula_input("id", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("type", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("range", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("damage", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("number_of_attacks", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("special", FORMULA_READ_ONLY));
}

int attack_type_callable::do_compare(const formula_callable* callable) const
{
	const attack_type_callable* att_callable = dynamic_cast<const attack_type_callable*>(callable);
	if(att_callable == NULL) {
		return formula_callable::do_compare(callable);
	}

	if (att_.damage() != att_callable->att_.damage() )
		return att_.damage() - att_callable->att_.damage();

	if (att_.num_attacks() != att_callable->att_.num_attacks() )
		return att_.num_attacks() - att_callable->att_.num_attacks();

	if ( att_.id() != att_callable->att_.id() )
		return att_.id().compare(att_callable->att_.id());

	if ( att_.type() != att_callable->att_.type() )
		return att_.type().compare(att_callable->att_.type());

	if ( att_.range() != att_callable->att_.range() )
		return att_.range().compare(att_callable->att_.range());

	return att_.weapon_specials().compare(att_callable->att_.weapon_specials());
}

variant unit_callable::get_value(const std::string& key) const
{
	if(key == "x") {
		if (loc_==map_location::null_location()) {
			return variant();
		} else {
			return variant(loc_.x+1);
		}
	} else if(key == "y") {
		if (loc_==map_location::null_location()) {
			return variant();
		} else {
			return variant(loc_.y+1);
		}
	} else if(key == "loc") {
		if (loc_==map_location::null_location()) {
			return variant();
		} else {
			return variant(new location_callable(loc_));
		}
	} else if(key == "id") {
		return variant(u_.id());
	} else if(key == "type") {
		return variant(u_.type_id());
	} else if(key == "name") {
		return variant(u_.name());
	} else if(key == "usage") {
		return variant(u_.usage());
	} else if(key == "leader" || key == "canrecruit") {
		return variant(u_.can_recruit());
	} else if(key == "undead") {
		return variant(u_.get_state("not_living") ? 1 : 0);
	} else if(key == "attacks") {
		const std::vector<attack_type>& att = u_.attacks();
		std::vector<variant> res;

		for( std::vector<attack_type>::const_iterator i = att.begin(); i != att.end(); ++i)
			res.push_back(variant(new attack_type_callable(*i)));
		return variant(&res);
	} else if(key == "abilities") {
		std::vector<std::string> abilities = u_.get_ability_list();
		std::vector<variant> res;

		if (abilities.empty())
			return variant( &res );

		for (std::vector<std::string>::iterator it = abilities.begin(); it != abilities.end(); ++it)
		{
			res.push_back( variant(*it) );
		}
		return variant( &res );
	} else if(key == "hitpoints") {
		return variant(u_.hitpoints());
	} else if(key == "max_hitpoints") {
		return variant(u_.max_hitpoints());
	} else if(key == "experience") {
		return variant(u_.experience());
	} else if(key == "max_experience") {
		return variant(u_.max_experience());
	} else if(key == "level" || key == "full") {
		// This allows writing "upkeep == full"
		return variant(u_.level());
	} else if(key == "total_movement" || key == "max_moves") {
		return variant(u_.total_movement());
	} else if(key == "movement_left" || key == "moves") {
		return variant(u_.movement_left());
	} else if(key == "attacks_left") {
		return variant(u_.attacks_left());
	} else if(key == "max_attacks") {
		return variant(u_.max_attacks());
	} else if(key == "traits") {
		const std::vector<std::string> traits = u_.get_traits_list();
		std::vector<variant> res;

		if(traits.empty())
			return variant( &res );

		for (std::vector<std::string>::const_iterator it = traits.begin(); it != traits.end(); ++it)
		{
			res.push_back( variant(*it) );
		}
		return variant( &res );
	} else if(key == "extra_recruit") {
		const std::vector<std::string> recruits = u_.recruits();
		std::vector<variant> res;

		if(recruits.empty())
			return variant( &res );

		for (std::vector<std::string>::const_iterator it = recruits.begin(); it != recruits.end(); ++it)
		{
			res.push_back( variant(*it) );
		}
		return variant( &res );
	} else if(key == "advances_to") {
		const std::vector<std::string> advances = u_.advances_to();
		std::vector<variant> res;

		if(advances.empty())
			return variant( &res );

		for (std::vector<std::string>::const_iterator it = advances.begin(); it != advances.end(); ++it)
		{
			res.push_back( variant(*it) );
		}
		return variant( &res );
	} else if(key == "states" || key == "status") {
		const std::map<std::string, std::string>& states_map = u_.get_states();

		return convert_map( states_map );
	} else if(key == "side") {
		return variant(u_.side()-1);
	} else if(key == "cost") {
		return variant(u_.cost());
	} else if(key == "upkeep") {
		return variant(u_.upkeep());
	} else if(key == "loyal") {
		// So we can write "upkeep == loyal"
		return variant(0);
	} else if(key == "hidden") {
		return variant(u_.get_hidden());
	} else if(key == "petrified") {
		return variant(u_.incapacitated());
	} else if(key == "resting") {
		return variant(u_.resting());
	} else if(key == "role") {
		return variant(u_.get_role());
	} else if(key == "race") {
		return variant(u_.race()->id());
	} else if(key == "gender") {
		return variant(gender_string(u_.gender()));
	} else if(key == "variation") {
		return variant(u_.variation());
	} else if(key == "zoc") {
		return variant(u_.get_emit_zoc());
	} else if(key == "alignment") {
		return variant(u_.alignment().to_string());
	} else if(key == "facing") {
		return variant(map_location::write_direction(u_.facing()));
	} else if(key == "vars") {
		if(u_.formula_manager().formula_vars()) {
			return variant(u_.formula_manager().formula_vars().get());
		} else {
			return variant();
		}
	} else if(key == "n" || key == "s" || key == "ne" || key == "se" || key == "nw" || key == "sw" || key == "lawful" || key == "neutral" || key == "chaotic" || key == "liminal" || key == "male" || key == "female") {
		return variant(key);
	} else {
		return variant();
	}
}

void unit_callable::get_inputs(std::vector<game_logic::formula_input>* inputs) const
{
	using game_logic::FORMULA_READ_ONLY;
	inputs->push_back(game_logic::formula_input("x", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("y", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("loc", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("id", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("type", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("name", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("canrecruit", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("undead", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("traits", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("attacks", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("abilities", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("hitpoints", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("max_hitpoints", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("experience", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("max_experience", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("level", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("moves", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("max_moves", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("attacks_left", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("max_attacks", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("side", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("extra_recruit", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("advances_to", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("status", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("cost", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("usage", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("upkeep", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("hidden", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("petrified", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("resting", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("role", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("race", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("gender", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("variation", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("zoc", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("alignment", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("facing", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("vars", FORMULA_READ_ONLY));
}

int unit_callable::do_compare(const formula_callable* callable) const
{
	const unit_callable* u_callable = dynamic_cast<const unit_callable*>(callable);
	if(u_callable == NULL) {
		return formula_callable::do_compare(callable);
	}

	return u_.underlying_id() - u_callable->u_.underlying_id();
}

variant unit_type_callable::get_value(const std::string& key) const
{
	if(key == "id") {
		return variant(u_.id());
	} else if(key == "type") {
		return variant(u_.type_name());
	} else if(key == "alignment") {
		return variant(lexical_cast<std::string>(u_.alignment()));
	} else if(key == "abilities") {
		std::vector<std::string> abilities = u_.get_ability_list();
		std::vector<variant> res;

		if (abilities.empty())
			return variant( &res );

		for (std::vector<std::string>::iterator it = abilities.begin(); it != abilities.end(); ++it)
		{
			res.push_back( variant(*it) );
		}
		return variant( &res );
	} else if(key == "attacks") {
		std::vector<attack_type> att = u_.attacks();
		std::vector<variant> res;

		for( std::vector<attack_type>::iterator i = att.begin(); i != att.end(); ++i)
			res.push_back(variant(new attack_type_callable(*i)));
		return variant(&res);
	} else if(key == "hitpoints") {
		return variant(u_.hitpoints());
	} else if(key == "experience") {
		return variant(u_.experience_needed(true));
	} else if(key == "level") {
		return variant(u_.level());
	} else if(key == "total_movement") {
		return variant(u_.movement());
	} else if(key == "unpoisonable") {
		return variant(u_.musthave_status("unpoisonable"));
	} else if(key == "undrainable") {
		return variant(u_.musthave_status("undrainable"));
	} else if(key == "unplagueable") {
		return variant(u_.musthave_status("unplagueable"));
	} else if(key == "cost") {
		return variant(u_.cost());
	} else if(key == "usage") {
		return variant(u_.usage());
	} else {
		return variant();
	}
}

void unit_type_callable::get_inputs(std::vector<game_logic::formula_input>* inputs) const
{
	using game_logic::FORMULA_READ_ONLY;
	inputs->push_back(game_logic::formula_input("id", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("type", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("alignment", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("abilities", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("attacks", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("hitpoints", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("experience", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("level", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("total_movement", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("undead", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("cost", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("usage", FORMULA_READ_ONLY));
}

int unit_type_callable::do_compare(const formula_callable* callable) const
{
	const unit_type_callable* u_callable = dynamic_cast<const unit_type_callable*>(callable);
	if(u_callable == NULL) {
		return formula_callable::do_compare(callable);
	}

	return u_.id().compare(u_callable->u_.id());
}

variant terrain_callable::get_value(const std::string& key) const
{
	if(key == "x") {
		return variant(loc_.x+1);
	} else if(key == "y") {
		return variant(loc_.y+1);
	} else if(key == "loc") {
		return variant(new location_callable(loc_));
	} else if(key == "id") {
		return variant(std::string(t_.id()));
	} else
		return variant();
}

void terrain_callable::get_inputs(std::vector<game_logic::formula_input>* inputs) const
{
	using game_logic::FORMULA_READ_ONLY;
	inputs->push_back(game_logic::formula_input("x", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("y", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("loc", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("id", FORMULA_READ_ONLY));
}

int terrain_callable::do_compare(const formula_callable* callable) const
{
	const terrain_callable* terr_callable = dynamic_cast<const terrain_callable*>(callable);
	if(terr_callable == NULL) {
		return formula_callable::do_compare(callable);
	}

	const map_location& other_loc = terr_callable->loc_;

	return loc_.do_compare(other_loc);
}



