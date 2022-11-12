/* $Id$ */
/*
   Copyright (C) 2006 by Dominic Bolin <dominic.bolin@exong.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "unit.hpp"
#include "unit_map.hpp"
#include "unit_abilities.hpp"

#include "wassert.hpp"
#include "log.hpp"
#include "pathutils.hpp"

#include <deque>

/*
 *
 * [abilities]
 * ...
 *
 * [heals]
 *	value=4
 *	max_value=8
 *	cumulative=no
 *	affect_allies=yes
 *	name= _ "heals"
// *	name_inactive=null
 *	description=  _ "Heals:
Allows the unit to heal adjacent friendly units at the beginning of each turn.

A unit cared for by a healer may heal up to 4 HP per turn.
A poisoned unit cannot be cured of its poison by a healer, and must seek the care of a village or a unit that can cure."
// *	description_inactive=null
 *	icon="misc/..."
// *	icon_inactive=null
 *	[adjacent_description]
 *		name= _ "heals"
// *		name_inactive=null
 *		description=  _ "Heals:
Allows the unit to heal adjacent friendly units at the beginning of each turn.

A unit cared for by a healer may heal up to 4 HP per turn.
A poisoned unit cannot be cured of its poison by a healer, and must seek the care of a village or a unit that can cure."
// *		description_inactive=null
 *		icon="misc/..."
// *		icon_inactive=null
 *	[/adjacent_description]
 *
 *	affect_self=yes
 *	[filter] // SUF
 *		...
 *	[/filter]
 *	[filter_location]
 *		terrain=f
 *		tod=lawful
 *	[/filter_location]
 *	[filter_self] // SUF
 *		...
 *	[/filter_self]
 *	[filter_adjacent] // SUF
 *		adjacent=n,ne,nw
 *		...
 *	[/filter_adjacent]
 *	[filter_adjacent_location]
 *		adjacent=n,ne,nw
 *		...
 *	[/filter_adjacent]
 *	[affect_adjacent]
 *		adjacent=n,ne,nw
 *		[filter] // SUF
 *			...
 *		[/filter]
 *	[/affect_adjacent]
 *	[affect_adjacent]
 *		adjacent=s,se,sw
 *		[filter] // SUF
 *			...
 *		[/filter]
 *	[/affect_adjacent]
 *
 * [/heals]
 *
 * ...
 * [/abilities]
 *
 */


namespace unit_abilities {

bool affects_side(const config& cfg, const std::vector<team>& teams, size_t side, size_t other_side)
{
	return ((cfg["affect_allies"] == "" || utils::string_bool(cfg["affect_allies"])) && side == other_side)
			|| (utils::string_bool(cfg["affect_allies"]) && !teams[side-1].is_enemy(other_side))
			|| (utils::string_bool(cfg["affect_enemies"]) && teams[side-1].is_enemy(other_side));
}

}


bool unit::get_ability_bool(const std::string& ability, const gamemap::location& loc) const
{
	const config* abilities = cfg_.child("abilities");
	if(abilities) {
		const config::child_list& list = abilities->get_children(ability);
		for(config::child_list::const_iterator i = list.begin(); i != list.end(); ++i) {
			if(ability_active(ability,**i,loc) && ability_affects_self(ability,**i,loc)) {
				return true;
			}
		}
	}

	wassert(units_ != NULL);
	wassert(teams_ != NULL);
	gamemap::location adjacent[6];
	get_adjacent_tiles(loc,adjacent);
	for(int i = 0; i != 6; ++i) {
		const unit_map::const_iterator it = units_->find(adjacent[i]);
		if(it != units_->end() &&
					!it->second.incapacitated()) {
			const config* adj_abilities = it->second.cfg_.child("abilities");
			if(adj_abilities) {
				const config::child_list& list = adj_abilities->get_children(ability);
				for(config::child_list::const_iterator j = list.begin(); j != list.end(); ++j) {
					if(unit_abilities::affects_side(**j,*teams_,side(),it->second.side())
						&& it->second.ability_active(ability,**j,adjacent[i]) && ability_affects_adjacent(ability,**j,i,loc)) {
						return true;
					}
				}
			}
		}
	}


	return false;
}
unit_ability_list unit::get_abilities(const std::string& ability, const gamemap::location& loc) const
{
	unit_ability_list res;

	const config* abilities = cfg_.child("abilities");
	if(abilities) {
		const config::child_list& list = abilities->get_children(ability);
		for(config::child_list::const_iterator i = list.begin(); i != list.end(); ++i) {
			if(ability_active(ability,**i,loc) && ability_affects_self(ability,**i,loc)) {
				res.cfgs.push_back(std::pair<config*,gamemap::location>(*i,loc));
			}
		}
	}

	wassert(units_ != NULL);
	gamemap::location adjacent[6];
	get_adjacent_tiles(loc,adjacent);
	for(int i = 0; i != 6; ++i) {
		const unit_map::const_iterator it = units_->find(adjacent[i]);
		if(it != units_->end() &&
		!it->second.incapacitated()) {
			const config* adj_abilities = it->second.cfg_.child("abilities");
			if(adj_abilities) {
				const config::child_list& list = adj_abilities->get_children(ability);
				for(config::child_list::const_iterator j = list.begin(); j != list.end(); ++j) {
					if(unit_abilities::affects_side(**j,*teams_,side(),it->second.side())
						&& it->second.ability_active(ability,**j,adjacent[i]) && ability_affects_adjacent(ability,**j,i,loc)) {
						res.cfgs.push_back(std::pair<config*,gamemap::location>(*j,adjacent[i]));
					}
				}
			}
		}
	}


	return res;
}

std::vector<std::string> unit::unit_ability_tooltips() const
{
	std::vector<std::string> res;

	const config* abilities = cfg_.child("abilities");
	if(abilities) {
		const config::child_map& list_map = abilities->all_children();
		for(config::child_map::const_iterator i = list_map.begin(); i != list_map.end(); ++i) {
			for(config::child_list::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {
				if((**j)["name"] != "") {
					res.push_back((**j)["name"].str());
					res.push_back((**j)["description"].str());
				}
			}
		}
	}
	return res;
}

std::vector<std::string> unit::ability_tooltips(const gamemap::location& loc) const
{
	std::vector<std::string> res;

	const config* abilities = cfg_.child("abilities");
	if(abilities) {
		const config::child_map& list_map = abilities->all_children();
		for(config::child_map::const_iterator i = list_map.begin(); i != list_map.end(); ++i) {
			for(config::child_list::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {
				if(ability_active(i->first,**j,loc)) {
					if((**j)["name"] != "") {
						res.push_back((**j)["name"].str());
						res.push_back((**j)["description"].str());
					}
				} else {
					if((**j)["name_inactive"] != "") {
						res.push_back((**j)["name_inactive"].str());
						res.push_back((**j)["description_inactive"].str());
					}
				}
			}
		}
	}

	wassert(units_ != NULL);
	gamemap::location adjacent[6];
	get_adjacent_tiles(loc,adjacent);
	for(int i = 0; i != 6; ++i) {
		const unit_map::const_iterator it = units_->find(adjacent[i]);
		if(it != units_->end() && 0 &&
		!it->second.incapacitated()) {
			const config* adj_abilities = it->second.cfg_.child("abilities");
			if(adj_abilities) {
				const config::child_map& adj_list_map = adj_abilities->all_children();
				for(config::child_map::const_iterator k = adj_list_map.begin(); k != adj_list_map.end(); ++k) {
					for(config::child_list::const_iterator j = k->second.begin(); j != k->second.end(); ++j) {
						if(unit_abilities::affects_side(**j,*teams_,side(),it->second.side())) {
							const config* adj_desc = (*j)->child("adjacent_description");
							if(ability_affects_adjacent(k->first,**j,i,adjacent[i])) {
								if(!adj_desc) {
									if(it->second.ability_active(k->first,**j,loc)) {
										if((**j)["name"] != "") {
											res.push_back((**j)["name"].str());
											res.push_back((**j)["description"].str());
										}
									} else {
										if((**j)["name_inactive"] != "") {
											res.push_back((**j)["name_inactive"].str());
											res.push_back((**j)["description_inactive"].str());
										}
									}
								} else {
									if(it->second.ability_active(k->first,**j,loc)) {
										if((*adj_desc)["name"] != "") {
											res.push_back((*adj_desc)["name"].str());
											res.push_back((*adj_desc)["description"].str());
										}
									} else {
										if((*adj_desc)["name_inactive"] != "") {
											res.push_back((*adj_desc)["name_inactive"].str());
											res.push_back((*adj_desc)["description_inacive"].str());
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}


	return res;
}

/*
 *
 * cfg: an ability WML structure
 *
 */

bool unit::ability_active(const std::string& ability,const config& cfg,const gamemap::location& loc) const
{
	if(cfg.child("filter_self") != NULL) {
		if(!matches_filter(*cfg.child("filter_self"),loc,ability=="illuminates")) {
			return false;
		}
	}
	wassert(units_ != NULL);
	gamemap::location adjacent[6];
	get_adjacent_tiles(loc,adjacent);
	gamemap::location::DIRECTION index=gamemap::location::NDIRECTIONS;
	const config::child_list& adj_filt = cfg.get_children("filter_adjacent");
	config::child_list::const_iterator i;
	for(i = adj_filt.begin(); i != adj_filt.end(); ++i) {
		std::vector<std::string> dirs = utils::split((**i)["adjacent"]);
		if(dirs.size()==1 && dirs.front()=="") {
		} else {
			for(std::vector<std::string>::const_iterator j = dirs.begin(); j != dirs.end(); ++j) {
				index = gamemap::location::parse_direction(*j);
				if(index != gamemap::location::NDIRECTIONS) {
					unit_map::const_iterator unit = units_->find(adjacent[index]);
					if(unit == units_->end()) {
						return false;
					}
					if(!unit->second.matches_filter(**i,unit->first,ability=="illuminates")) {
						return false;
					}
				}
			}
		}
	}
	index=gamemap::location::NDIRECTIONS;
	const config::child_list& adj_filt_loc = cfg.get_children("filter_adjacent_location");
	for(i = adj_filt_loc.begin(); i != adj_filt_loc.end(); ++i) {
		std::vector<std::string> dirs = utils::split((**i)["adjacent"]);
		if(dirs.size()==1 && dirs.front()=="") {
		} else {
			for(std::vector<std::string>::const_iterator j = dirs.begin(); j != dirs.end(); ++j) {
				index = gamemap::location::parse_direction(*j);
				if(index != gamemap::location::NDIRECTIONS) {
					wassert(map_ != NULL);
					wassert(gamestatus_ != NULL);
					if(!map_->terrain_matches_filter(adjacent[index],**i,*gamestatus_,*units_,ability=="illuminates")) {
						return false;
					}
				}
			}
		}
	}
	return true;
}
/*
 *
 * cfg: an ability WML structure
 *
 */
bool unit::ability_affects_adjacent(const std::string& ability,const config& cfg,int dir,const gamemap::location& loc) const
{
	wassert(dir >=0 && dir <= 5);
	static const std::string adjacent_names[6] = {"n","ne","se","s","sw","nw"};
	const config::child_list& affect_adj = cfg.get_children("affect_adjacent");
	for(config::child_list::const_iterator i = affect_adj.begin(); i != affect_adj.end(); ++i) {
		std::vector<std::string> dirs = utils::split((**i)["adjacent"]);
		if(std::find(dirs.begin(),dirs.end(),adjacent_names[dir]) != dirs.end()) {
			if((*i)->child("filter") == NULL || matches_filter(*(*i)->child("filter"),loc,ability=="illuminates")) {
				return true;
			}
		}
	}
	return false;
}
/*
 *
 * cfg: an ability WML structure
 *
 */
bool unit::ability_affects_self(const std::string& ability,const config& cfg,const gamemap::location& loc) const
{
	if(cfg.child("filter")==NULL) {
		if(utils::string_bool(cfg["affect_self"],true)) {
			return true;
		} else {
			return false;
		}
	}
	if(utils::string_bool(cfg["affect_self"],true)) {
		return matches_filter(*cfg.child("filter"),loc,ability=="illuminates");
	} else {
		return false;
	}
}

bool unit::has_ability_type(const std::string& ability) const
{
	const config* list = cfg_.child("abilities");
	if(list) {
		return !list->get_children(ability).empty();
	}
	return false;
}


bool unit_ability_list::empty() const
{
	return cfgs.empty();
}

std::pair<int,gamemap::location> unit_ability_list::highest(const std::string& key, int def) const
{
	if(cfgs.empty()) {
		return std::make_pair(def,gamemap::location::null_location);
	}
	gamemap::location best_loc = gamemap::location::null_location;
	int abs_max = -10000;
	int flat = -10000;
	int stack = 0;
	for(std::vector<std::pair<config*,gamemap::location> >::const_iterator i = cfgs.begin(); i != cfgs.end(); ++i) {
		if(utils::string_bool((*i->first)["cumulative"])) {
			stack += lexical_cast_default<int>((*i->first)[key]);
			if(lexical_cast_default<int>((*i->first)[key]) > abs_max) {
				abs_max = lexical_cast_default<int>((*i->first)[key]);
				best_loc = i->second;
			}
		} else {
			int val = (*i->first)[key] != "" ? lexical_cast_default<int>((*i->first)[key]) : def;
			flat = maximum<int>(flat,val);
			if(lexical_cast_default<int>((*i->first)[key]) > abs_max) {
				abs_max = lexical_cast_default<int>((*i->first)[key]);
				best_loc = i->second;
			}
		}
	}
	return std::pair<int,gamemap::location>(flat + stack,best_loc);
}
std::pair<int,gamemap::location> unit_ability_list::lowest(const std::string& key, int def) const
{
	if(cfgs.empty()) {
		return std::make_pair(def,gamemap::location::null_location);
	}
	gamemap::location best_loc = gamemap::location::null_location;
	int abs_max = 10000;
	int flat = 10000;
	int stack = 0;
	for(std::vector<std::pair<config*,gamemap::location> >::const_iterator i = cfgs.begin(); i != cfgs.end(); ++i) {
		if(utils::string_bool((*i->first)["cumulative"])) {
			stack += lexical_cast_default<int>((*i->first)[key]);
			if(lexical_cast_default<int>((*i->first)[key]) < abs_max) {
				abs_max = lexical_cast_default<int>((*i->first)[key]);
				best_loc = i->second;
			}
		} else {
			int val = (*i->first)[key] != "" ? lexical_cast_default<int>((*i->first)[key]) : def;
			flat = minimum<int>(flat,val);
			if(lexical_cast_default<int>((*i->first)[key]) < abs_max) {
				abs_max = lexical_cast_default<int>((*i->first)[key]);
				best_loc = i->second;
			}
		}
	}
	return std::pair<int,gamemap::location>(flat + stack,best_loc);
}
























/*
 *
 * [special]
 * [swarm]
 *	name= _ "swarm"
 *	name_inactive= _ ""
 *	description= _ ""
 *	description_inactive= _ ""
 *	cumulative=no
 *	apply_to=self  #self,opponent,defender,attacker
 *	#active_on=defend  .. offense
 *
 *	attacks_max=4
 *	attacks_min=2
 *
 *	[filter_self] // SUF
 *		...
 *	[/filter_self]
 *	[filter_opponent] // SUF
 *	[filter_attacker] // SUF
 *	[filter_defender] // SUF
 *	[filter_adjacent] // SAUF
 *	[filter_adjacent_location] // SAUF + locs
 * [/swarm]
 * [/special]
 *
 */






bool attack_type::get_special_bool(const std::string& special,bool force) const
{
//	log_scope("get_special_bool");
	const config* specials = cfg_.child("specials");
	if(specials) {
		const config::child_list& list = specials->get_children(special);
		for(config::child_list::const_iterator i = list.begin(); i != list.end(); ++i) {
			if(force || special_active(**i,true)) {
				return true;
			}
		}
	}
	if(!force && other_attack_ != NULL) {
		specials = other_attack_->cfg_.child("specials");
		if(specials) {
			const config::child_list& list = specials->get_children(special);
			for(config::child_list::const_iterator i = list.begin(); i != list.end(); ++i) {
				if(other_attack_->special_active(**i,false)) {
					return true;
				}
			}
		}
	}
	return false;
}
unit_ability_list attack_type::get_specials(const std::string& special) const
{
//	log_scope("get_specials");
	unit_ability_list res;
	const config* specials = cfg_.child("specials");
	if(specials) {
		const config::child_list& list = specials->get_children(special);
		for(config::child_list::const_iterator i = list.begin(); i != list.end(); ++i) {
			if(special_active(**i,true)) {
				res.cfgs.push_back(std::pair<config*,gamemap::location>(*i,attacker_ ? aloc_ : dloc_));
			}
		}
	}
	if(other_attack_ != NULL) {
		specials = other_attack_->cfg_.child("specials");
		if(specials) {
			const config::child_list& list = specials->get_children(special);
			for(config::child_list::const_iterator i = list.begin(); i != list.end(); ++i) {
				if(other_attack_->special_active(**i,false)) {
					res.cfgs.push_back(std::pair<config*,gamemap::location>(*i,attacker_ ? dloc_ : aloc_));
				}
			}
		}
	}
	return res;
}
std::vector<std::string> attack_type::special_tooltips(bool force) const
{
//	log_scope("special_tooltips");
	std::vector<std::string> res;
	const config* specials = cfg_.child("specials");
	if(specials) {
		const config::child_map& list_map = specials->all_children();
		for(config::child_map::const_iterator i = list_map.begin(); i != list_map.end(); ++i) {
			for(config::child_list::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {
				if(force || special_active(**j,true)) {
					if((**j)["name"] != "") {
						res.push_back((**j)["name"]);
						res.push_back((**j)["description"]);
					}
				} else {
					if((**j)["name_inactive"] != "") {
						res.push_back((**j)["name_inactive"]);
						res.push_back((**j)["description_inactive"]);
					}
				}
			}
		}
	}
	return res;
}
std::string attack_type::weapon_specials(bool force) const
{
//	log_scope("weapon_specials");
	std::string res;
	const config* specials = cfg_.child("specials");
	if(specials) {
		const config::child_map& list_map = specials->all_children();
		for(config::child_map::const_iterator i = list_map.begin(); i != list_map.end(); ++i) {
			for(config::child_list::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {
				if(force || special_active(**j,true,true)) {
					if((**j)["name"] != "") {
						res += (**j)["name"];
						res += ",";
					}
				} else {
					if((**j)["name_inactive"] != "") {
						res += (**j)["name_inactive"];
						res += ",";
					}
				}
			}
		}
	}

	return res.substr(0,res.size()-1);
}



/*
 *
 * cfg: a weapon special WML structure
 *
 */
bool attack_type::special_active(const config& cfg,bool self,bool report) const
{
//	log_scope("special_active");
	wassert(unitmap_ != NULL);
	unit_map::const_iterator att = unitmap_->find(aloc_);
	unit_map::const_iterator def = unitmap_->find(dloc_);

	if(self) {
		if(!special_affects_self(cfg)) {
			return false;
		}
	} else {
		if(!special_affects_opponent(cfg)) {
			return false;
		}
	}

	if(attacker_) {
		if(!report && cfg["active_on"] != "" && cfg["active_on"] != "offense") {
			return false;
		}
		if(cfg.child("filter_self") != NULL) {
			if(att == unitmap_->end() || !att->second.matches_filter(*cfg.child("filter_self"),aloc_)) {
				return false;
			}
			if(cfg.child("filter_self")->child("filter_weapon") != NULL) {
				if(!matches_filter(*cfg.child("filter_self")->child("filter_weapon"),true)) {
					return false;
				}
			}
		}
		if(cfg.child("filter_opponent") != NULL) {
			if(def == unitmap_->end() || !def->second.matches_filter(*cfg.child("filter_opponent"),dloc_)) {
				return false;
			}

			if(cfg.child("filter_opponent")->child("filter_weapon") != NULL) {
				if(!other_attack_ || !other_attack_->matches_filter(*cfg.child("filter_opponent")->child("filter_weapon"),true)) {
					return false;
				}
			}
		}
	} else {
		if(!report && cfg["active_on"] != "" && cfg["active_on"] != "defense") {
			return false;
		}
		if(cfg.child("filter_self") != NULL) {
			if(def == unitmap_->end() || !def->second.matches_filter(*cfg.child("filter_self"),dloc_)) {
				return false;
			}
			if(cfg.child("filter_self")->child("filter_weapon") != NULL) {
				if(!matches_filter(*cfg.child("filter_self")->child("filter_weapon"),true)) {
					return false;
				}
			}
		}
		if(cfg.child("filter_opponent") != NULL) {
			if(att == unitmap_->end() || !att->second.matches_filter(*cfg.child("filter_opponent"),aloc_)) {
				return false;
			}
			if(cfg.child("filter_opponent")->child("filter_weapon") != NULL) {
				if(!other_attack_ || !other_attack_->matches_filter(*cfg.child("filter_opponent")->child("filter_weapon"),true)) {
					return false;
				}
			}
		}
	}
	if(cfg.child("filter_attacker") != NULL) {
		if(att == unitmap_->end() || !att->second.matches_filter(*cfg.child("filter_attacker"),aloc_)) {
			return false;
		}
		if(attacker_) {
			if(cfg.child("filter_attacker")->child("filter_weapon") != NULL) {
				if(!matches_filter(*cfg.child("filter_attacker")->child("filter_weapon"),true)) {
					return false;
				}
			}
		} else {
			if(cfg.child("filter_attacker")->child("filter_weapon") != NULL) {
				if(!other_attack_ || !other_attack_->matches_filter(*cfg.child("filter_attacker")->child("filter_weapon"),true)) {
					return false;
				}
			}
		}
	}
	if(cfg.child("filter_defender") != NULL) {
		if(def == unitmap_->end() || !def->second.matches_filter(*cfg.child("filter_defender"),dloc_)) {
			return false;
		}
		if(!attacker_) {
			if(cfg.child("filter_defender")->child("filter_weapon") != NULL) {
				if(!matches_filter(*cfg.child("filter_defender")->child("filter_weapon"),true)) {
					return false;
				}
			}
		} else {
			if(cfg.child("filter_defender")->child("filter_weapon") != NULL) {
				if(!other_attack_ || !other_attack_->matches_filter(*cfg.child("filter_defender")->child("filter_weapon"),true)) {
					return false;
				}
			}
		}
	}
	gamemap::location adjacent[6];
	if(attacker_) {
		get_adjacent_tiles(aloc_,adjacent);
	} else {
		get_adjacent_tiles(dloc_,adjacent);
	}
	gamemap::location::DIRECTION index=gamemap::location::NDIRECTIONS;
	const config::child_list& adj_filt = cfg.get_children("filter_adjacent");
	config::child_list::const_iterator i;
	for(i = adj_filt.begin(); i != adj_filt.end(); ++i) {
		std::vector<std::string> dirs = utils::split((**i)["adjacent"]);
		if(dirs.size()==1 && dirs.front()=="") {
		} else {
			for(std::vector<std::string>::const_iterator j = dirs.begin(); j != dirs.end(); ++j) {
				index = gamemap::location::parse_direction(*j);
				if(index != gamemap::location::NDIRECTIONS) {
					unit_map::const_iterator unit = unitmap_->find(adjacent[index]);
					if(unit == unitmap_->end()) {
						return false;
					}
					if(!unit->second.matches_filter(**i,unit->first)) {
						return false;
					}
				}
			}
		}
	}
	index=gamemap::location::NDIRECTIONS;
	const config::child_list& adj_filt_loc = cfg.get_children("filter_adjacent_location");
	for(i = adj_filt_loc.begin(); i != adj_filt_loc.end(); ++i) {
		std::vector<std::string> dirs = utils::split((**i)["adjacent"]);
		if(dirs.size()==1 && dirs.front()=="") {
		} else {
			for(std::vector<std::string>::const_iterator j = dirs.begin(); j != dirs.end(); ++j) {
				index = gamemap::location::parse_direction(*j);
				if(index != gamemap::location::NDIRECTIONS) {
					wassert(map_ != NULL);
					wassert(game_status_ != NULL);
					if(!map_->terrain_matches_filter(adjacent[index],**i,*game_status_,*unitmap_)) {
						return false;
					}
				}
			}
		}
	}
	return true;
}
/*
 *
 * cfg: a weapon special WML structure
 *
 */
bool attack_type::special_affects_opponent(const config& cfg) const
{
//	log_scope("special_affects_opponent");
	if(cfg["apply_to"]=="both") {
		return true;
	}
	if(cfg["apply_to"]=="opponent") {
		return true;
	}
	if(cfg["apply_to"]!="") {
		if(attacker_ && cfg["apply_to"] == "defender") {
			return true;
		}
		if(!attacker_ && cfg["apply_to"] == "attacker") {
			return true;
		}
	}
	return false;
}
/*
 *
 * cfg: a weapon special WML structure
 *
 */
bool attack_type::special_affects_self(const config& cfg) const
{
//	log_scope("special_affects_self");
	if(cfg["apply_to"]=="both") {
		return true;
	}
	if(cfg["apply_to"]=="self" || cfg["apply_to"]=="") {
		return true;
	}
	if(attacker_ && cfg["apply_to"] == "attacker") {
		return true;
	}
	if(!attacker_ && cfg["apply_to"] == "defender") {
		return true;
	}
	return false;
}
void attack_type::set_specials_context(const gamemap::location& aloc,const gamemap::location& dloc,
                              const game_data* gamedata, const unit_map* unitmap,
							  const gamemap* map, const gamestatus* game_status,
							  const std::vector<team>* teams, bool attacker,const attack_type* other_attack) const
{
	aloc_ = aloc;
	dloc_ = dloc;
	gamedata_ = gamedata;
	unitmap_ = unitmap;
	map_ = map;
	game_status_ = game_status;
	teams_ = teams;
	attacker_ = attacker;
	other_attack_ = other_attack;
}

void attack_type::set_specials_context(const gamemap::location& loc,const unit& un) const
{
	aloc_ = loc;
	dloc_ = gamemap::location();
	gamedata_ = un.gamedata_;
	unitmap_ = un.units_;
	map_ = un.map_;
	game_status_ = un.gamestatus_;
	teams_ = un.teams_;
	attacker_ = true;
	other_attack_ = NULL;
}




namespace unit_abilities
{


individual_effect::individual_effect(value_modifier t,int val,config* abil,const gamemap::location& l)
{
	set(t,val,abil,l);
}
void individual_effect::set(value_modifier t,int val,config* abil,const gamemap::location& l)
{
	type=t;
	value=val;
	ability=abil;
	loc=l;
}



effect::effect(const unit_ability_list& list, int def, bool backstab)
{

	int value_set = def; bool value_is_set = false;
	std::map<std::string,individual_effect> values_add;
	std::map<std::string,individual_effect> values_mul;

	individual_effect set_effect(NOT_USED,0,NULL,gamemap::location());

	for(std::vector<std::pair<config*,gamemap::location> >::const_iterator i = list.cfgs.begin(); i != list.cfgs.end(); ++i) {
		const config& cfg = (*i->first);
		const std::string& effect_id = cfg["id"] != "" ? cfg["id"] : cfg["name"];

		if(utils::string_bool(cfg["backstab"]) && !backstab) {
			continue;
		}
		const config* const apply_filter = cfg.child("filter_base_value");
		if(apply_filter) {
			if((*apply_filter)["equals"] != "" && lexical_cast_default<int>((*apply_filter)["equals"]) != def) {
				continue;
			}
			if((*apply_filter)["not_equals"] != "" && lexical_cast_default<int>((*apply_filter)["not_equals"]) == def) {
				continue;
			}
			if((*apply_filter)["less_than"] != "" && lexical_cast_default<int>((*apply_filter)["less_than"]) < def) {
				continue;
			}
			if((*apply_filter)["greater_than"] != "" && lexical_cast_default<int>((*apply_filter)["greater_than"]) > def) {
				continue;
			}
			if((*apply_filter)["greater_than_equal_to"] != "" && lexical_cast_default<int>((*apply_filter)["greater_than_equal_to"]) >= def) {
				continue;
			}
			if((*apply_filter)["less_than_equal_to"] != "" && lexical_cast_default<int>((*apply_filter)["less_than_equal_to"]) <= def) {
				continue;
			}
		}
		int value = lexical_cast_default<int>(cfg["value"]);
		int add = lexical_cast_default<int>(cfg["add"]);
		int multiply = static_cast<int>(lexical_cast_default<float>(cfg["multiply"])*100);

		if(!value_is_set && !utils::string_bool(cfg["cumulative"]) && cfg["value"] != "") {
			value_is_set = true;
			value_set = value;
			set_effect.set(SET,value,i->first,i->second);
		} else if(cfg["value"] != "") {
			value_is_set = true;
			if(utils::string_bool(cfg["cumulative"])) {
				value_set = maximum<int>(value_set,def);
			}
			if(value > value_set) {
				value_set = value;
				set_effect.set(SET,value,i->first,i->second);
			}
		}
		if(cfg["add"] != "") {
			std::map<std::string,individual_effect>::iterator add_effect = values_add.find(effect_id);
			if(add_effect == values_add.end() || add > add_effect->second.value) {
				values_add[effect_id].set(ADD,add,i->first,i->second);
			}
		}
		if(cfg["multiply"] != "") {
			std::map<std::string,individual_effect>::iterator mul_effect = values_mul.find(effect_id);
			if(mul_effect == values_mul.end() || multiply > mul_effect->second.value) {
				values_mul[effect_id].set(MUL,multiply,i->first,i->second);
			}
		}
	}

	if(value_is_set && set_effect.type != NOT_USED) {
		effect_list_.push_back(set_effect);
	}

	int multiplier = 1;
	int divisor = 1;
	std::map<std::string,individual_effect>::const_iterator e;
	for(e = values_mul.begin(); e != values_mul.end(); ++e) {
		multiplier *= e->second.value;
		divisor *= 100;
		effect_list_.push_back(e->second);
	}
	int addition = 0;
	for(e = values_add.begin(); e != values_add.end(); ++e) {
		addition += e->second.value;
		effect_list_.push_back(e->second);
	}

	composite_value_ = (value_set + addition) * multiplier / divisor;

}



int effect::get_composite_value() const
{
	return composite_value_;
}

effect_list::const_iterator effect::begin() const
{
	return effect_list_.begin();
}

effect_list::const_iterator effect::end() const
{
	return effect_list_.end();
}





} // end namespace unit_abilities























