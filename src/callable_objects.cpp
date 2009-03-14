/* $Id$ */
/*
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "callable_objects.hpp"

template <typename T, typename K>
variant convert_map( const std::map<T, K>& input_map ) {
	std::map<variant,variant> tmp;

	for(typename std::map< T, K>::const_iterator i = input_map.begin(); i != input_map.end(); ++i) {
			tmp[ variant(i->first) ] = variant( i->second );
	}

	return variant( &tmp );
}

template <typename T>
variant convert_vector( const std::vector<T>& input_vector ) {
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
	if(other_loc.x != loc_.x) {
		return loc_.x - other_loc.x;
	}

	return loc_.y - other_loc.y;
}

void location_callable::serialize_to_string(std::string& str) const
{
	std::ostringstream s;
	s << "loc(" << (loc_.x+1) << "," << (loc_.y+1) << ")";
	str = s.str();
}

variant move_map_callable::get_value(const std::string& key) const
{
	using namespace game_logic;
	if(key == "moves") {
		std::vector<variant> vars;
		for(move_map::const_iterator i = srcdst_.begin(); i != srcdst_.end(); ++i) {
			move_callable* item = new move_callable(i->first, i->second);
			vars.push_back(variant(item));
		}

		return variant(&vars);
	} else if(key == "has_moves") {
		return variant(!srcdst_.empty());
	} else {
		return variant();
	}
}

void move_map_callable::get_inputs(std::vector<game_logic::formula_input>* inputs) const
{
	using game_logic::FORMULA_READ_ONLY;
	inputs->push_back(game_logic::formula_input("moves", FORMULA_READ_ONLY));
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
		std::string specials = att_.weapon_specials(true);

		if(specials == "") {
			std::vector<variant> res;
			return variant( &res );
		}

		std::vector< std::string > string_vector = utils::split( specials );

		return convert_vector( string_vector );
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

variant unit_callable::get_value(const std::string& key) const
{
	if(key == "x") {
		return variant(loc_.x+1);
	} else if(key == "y") {
		return variant(loc_.y+1);
	} else if(key == "loc") {
		return variant(new location_callable(loc_));
	} else if(key == "id") {
		return variant(u_.id());
	} else if(key == "type") {
		return variant(u_.type_id());
	} else if(key == "name") {
		return variant(u_.name());
	} else if(key == "leader") {
		return variant(u_.can_recruit());
	} else if(key == "undead") {
		if ( u_.get_state("not_living") == "yes" )
					return variant( 1 );

		return variant( 0 );
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
	} else if(key == "level") {
		return variant(u_.level());
	} else if(key == "total_movement") {
		return variant(u_.total_movement());
	} else if(key == "movement_left") {
		return variant(u_.movement_left());
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
	} else if(key == "states") {
		const std::map<std::string, std::string>& states_map = u_.get_states();

		return convert_map( states_map );
	} else if(key == "side") {
		return variant(u_.side()-1);
	} else if(key == "cost") {
		return variant(u_.cost());
	} else if(key == "vars") {
		if(u_.formula_vars()) {
			return variant(u_.formula_vars().get());
		} else {
			return variant();
		}
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
	inputs->push_back(game_logic::formula_input("leader", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("undead", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("traits", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("attacks", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("abilities", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("hitpoints", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("max_hitpoints", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("experience", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("max_experience", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("level", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("total_movement", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("movement_left", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("side", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("states", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("cost", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("vars", FORMULA_READ_ONLY));
}

variant unit_type_callable::get_value(const std::string& key) const
{
	if(key == "id") {
		return variant(u_.id());
	} else if(key == "type") {
		return variant(u_.type_name());
	} else if(key == "alignment") {
		return variant(u_.alignment_id(u_.alignment()));
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
	} else if(key == "undead") {
		return variant(u_.not_living());
	} else if(key == "cost") {
		return variant(u_.cost());
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

