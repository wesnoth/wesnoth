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

#include "wassert.hpp"
#include "log.hpp"
#include "pathutils.hpp"

typedef std::map<gamemap::location,unit> units_map;

/*
 *
 * [abilities]
 * ...
 * 
 * [heals]
 * 	value=4
 * 	max_value=8
 * 	cumulative=no
 * 	affect_allies=yes
 * 	name= _ "heals"
// * 	name_inactive=null
 * 	description=  _ "Heals:
Allows the unit to heal adjacent friendly units at the beginning of each turn.

A unit cared for by a healer may heal up to 4 HP per turn.
A poisoned unit cannot be cured of its poison by a healer, and must seek the care of a village or a unit that can cure."
// * 	description_inactive=null
 * 	icon="misc/..."
// * 	icon_inactive=null
 * 	[adjacent_description]
 * 		name= _ "heals"
// * 		name_inactive=null
 * 		description=  _ "Heals:
Allows the unit to heal adjacent friendly units at the beginning of each turn.

A unit cared for by a healer may heal up to 4 HP per turn.
A poisoned unit cannot be cured of its poison by a healer, and must seek the care of a village or a unit that can cure."
// * 		description_inactive=null
 * 		icon="misc/..."
// * 		icon_inactive=null
 * 	[/adjacent_description]
 * 	
 * 	affect_self=yes
 * 	[filter] // SUF
 * 		...
 * 	[/filter]
 * 	[filter_location]
 * 		terrain=f
 * 		tod=lawful
 * 	[/filter_location]
 * 	[filter_self] // SUF
 * 		...
 * 	[/filter_self]
 * 	[filter_adjacent] // SUF
 * 		adjacent=n,ne,nw
 * 		...
 * 	[/filter_adjacent]
 * 	[filter_adjacent_location]
 * 		adjacent=n,ne,nw
 * 		...
 * 	[/filter_adjacent]
 * 	[affect_adjacent]
 * 		adjacent=n,ne,nw
 * 		[filter] // SUF
 * 			...
 * 		[/filter]
 * 	[/affect_adjacent]
 * 	[affect_adjacent]
 * 		adjacent=s,se,sw
 * 		[filter] // SUF
 * 			...
 * 		[/filter]
 * 	[/affect_adjacent]
 * 	
 * [/heals]
 * 
 * ...
 * [/abilities]
 *
 */


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
					it->second.get_state("stoned") != "yes") {
			const config* adj_abilities = it->second.cfg_.child("abilities");
			if(adj_abilities) {
				const config::child_list& list = adj_abilities->get_children(ability);
				for(config::child_list::const_iterator j = list.begin(); j != list.end(); ++j) {
					if(((**j)["affect_allies"]=="yes" && !(*teams_)[side()-1].is_enemy(it->second.side())) 
						|| ((**j)["affect_enemies"]=="yes" && (*teams_)[side()-1].is_enemy(it->second.side())) &&
							ability_active(ability,**j,loc) && ability_affects_adjacent(ability,**j,i,loc)) {
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
		it->second.get_state("stoned") != "yes") {
			const config* adj_abilities = it->second.cfg_.child("abilities");
			if(adj_abilities) {
				const config::child_list& list = adj_abilities->get_children(ability);
				for(config::child_list::const_iterator j = list.begin(); j != list.end(); ++j) {
					if(((**j)["affect_allies"]=="yes" && !(*teams_)[side()-1].is_enemy(it->second.side())) 
						|| ((**j)["affect_enemies"]=="yes" && (*teams_)[side()-1].is_enemy(it->second.side())) &&
						ability_active(ability,**j,loc) && ability_affects_adjacent(ability,**j,i,loc)) {
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
		it->second.get_state("stoned") != "yes") {
			const config* adj_abilities = it->second.cfg_.child("abilities");
			if(adj_abilities) {
				const config::child_map& adj_list_map = adj_abilities->all_children();
				for(config::child_map::const_iterator k = adj_list_map.begin(); k != adj_list_map.end(); ++k) {
					for(config::child_list::const_iterator j = k->second.begin(); j != k->second.end(); ++j) {
					if(((**j)["affect_allies"]=="yes" && !(*teams_)[side()-1].is_enemy(it->second.side())) 
						|| ((**j)["affect_enemies"]=="yes" && (*teams_)[side()-1].is_enemy(it->second.side()))) {
							const config* adj_desc = (*j)->child("adjacent_description");
							if(ability_affects_adjacent(k->first,**j,i,loc)) {
								if(!adj_desc) {
									if(ability_active(k->first,**j,loc)) {
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
									if(ability_active(k->first,**j,loc)) {
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
	int index=-1;
	const config::child_list& adj_filt = cfg.get_children("filter_adjacent");
	config::child_list::const_iterator i;
	for(i = adj_filt.begin(); i != adj_filt.end(); ++i) {
		std::vector<std::string> dirs = utils::split((**i)["adjacent"]);
		if(dirs.size()==1 && dirs.front()=="") {	
		} else {
			for(std::vector<std::string>::const_iterator j = dirs.begin(); j != dirs.end(); ++j) {
				if(*j=="n") {
					index=0;
				} else if(*j=="ne") {
					index=1;
				} else if(*j=="se") {
					index=2;
				} else if(*j=="s") {
					index=3;
				} else if(*j=="sw") {
					index=4;
				} else if(*j=="nw") {
					index=5;
				} else {
					index=-1;
				}
				if(index != -1) {
					units_map::const_iterator unit = units_->find(adjacent[index]);
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
	index=-1;
	const config::child_list& adj_filt_loc = cfg.get_children("filter_adjacent_location");
	for(i = adj_filt_loc.begin(); i != adj_filt_loc.end(); ++i) {
		std::vector<std::string> dirs = utils::split((**i)["adjacent"]);
		if(dirs.size()==1 && dirs.front()=="") {	
		} else {
			for(std::vector<std::string>::const_iterator j = dirs.begin(); j != dirs.end(); ++j) {
				if(*j=="n") {
					index=0;
				} else if(*j=="ne") {
					index=1;
				} else if(*j=="se") {
					index=2;
				} else if(*j=="s") {
					index=3;
				} else if(*j=="sw") {
					index=4;
				} else if(*j=="nw") {
					index=5;
				} else {
					index=-1;
				}
				if(index != -1) {
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
//	wassert("not done" == "done");
	wassert(dir >=0 && dir <= 5);
	static const std::string adjacent_names[6] = {"n","ne","se","s","sw","nw"};
	const config::child_list& affect_adj = cfg.get_children("affect_adjacent");
	bool passed = false;
	for(config::child_list::const_iterator i = affect_adj.begin(); i != affect_adj.end(); ++i) {
		std::vector<std::string> dirs = utils::split((**i)["adjacent"]);
		if(std::find(dirs.begin(),dirs.end(),adjacent_names[dir]) != dirs.end()) {
			if((*i)->child("filter") != NULL) {
				if(matches_filter(*(*i)->child("filter"),loc,ability=="illuminates")) {
					passed = true;
				} else {
					return false;
				}
			} else {
				passed = true;
			}
		}
	}
	return passed;
}
/*
 *
 * cfg: an ability WML structure
 *
 */
bool unit::ability_affects_self(const std::string& ability,const config& cfg,const gamemap::location& loc) const
{
	if(cfg.child("filter")==NULL) {
		if(cfg["affect_self"] == "yes" || cfg["affect_self"] == "") {
			return true;
		} else {
			return false;
		}
	}
	if(cfg["affect_self"] == "yes" || cfg["affect_self"] == "") {
		return matches_filter(*cfg.child("filter"),loc,ability=="illuminates");
	} else {
		return false;
	}
}




bool unit_ability_list::empty() const
{
	return cfgs.empty();
}

std::pair<int,gamemap::location> unit_ability_list::highest(const std::string& key, int def) const
{
	gamemap::location best_loc;
	int abs_max = def;
	int flat = def;
	int stack = 0;
	for(std::vector<std::pair<config*,gamemap::location> >::const_iterator i = cfgs.begin(); i != cfgs.end(); ++i) {
		if((*i->first)["cumulative"]=="yes") {
			stack += lexical_cast_default<int>((*i->first)[key]);
			if(lexical_cast_default<int>((*i->first)[key]) > abs_max) {
				abs_max = lexical_cast_default<int>((*i->first)[key]);
				best_loc = i->second;
			}
		} else {
			flat = maximum<int>(flat,lexical_cast_default<int>((*i->first)[key]));
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
	gamemap::location best_loc;
	int abs_max = def;
	int flat = def;
	int stack = 0;
	for(std::vector<std::pair<config*,gamemap::location> >::const_iterator i = cfgs.begin(); i != cfgs.end(); ++i) {
		if((*i->first)["cumulative"]=="yes") {
			stack += lexical_cast_default<int>((*i->first)[key]);
			if(lexical_cast_default<int>((*i->first)[key]) < abs_max) {
				abs_max = lexical_cast_default<int>((*i->first)[key]);
				best_loc = i->second;
			}
		} else {
			flat = minimum<int>(flat,lexical_cast_default<int>((*i->first)[key]));
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
 * 	name= _ "swarm"
 * 	name_inactive= _ ""
 * 	description= _ ""
 * 	description_inactive= _ ""
 * 	cumulative=no
 * 	apply_to=self  #self,opponent,defender,attacker
 * 	#active_on=defend  .. offense
 * 	
 * 	attacks_max=4
 * 	attacks_min=2
 * 	
 * 	[filter_self] // SUF
 * 		...
 * 	[/filter_self]
 * 	[filter_opponent] // SUF
 * 	[filter_attacker] // SUF
 * 	[filter_defender] // SUF
 * 	[filter_adjacent] // SAUF
 * 	[filter_adjacent_location] // SAUF + locs
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
weapon_special_list attack_type::get_specials(const std::string& special) const
{
//	log_scope("get_specials");
	weapon_special_list res;
	const config* specials = cfg_.child("specials");
	if(specials) {
		const config::child_list& list = specials->get_children(special);
		for(config::child_list::const_iterator i = list.begin(); i != list.end(); ++i) {
			if(special_active(**i,true)) {
				res.cfgs.push_back(*i);
			}
		}
	}
	if(other_attack_ != NULL) {
		specials = other_attack_->cfg_.child("specials");
		if(specials) {
			const config::child_list& list = specials->get_children(special);
			for(config::child_list::const_iterator i = list.begin(); i != list.end(); ++i) {
				if(other_attack_->special_active(**i,false)) {
					res.cfgs.push_back(*i);
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
				if(!matches_filter(*cfg.child("filter_self")->child("filter_weapon"),0,true)) {
					return false;
				}
			}
		}
		if(cfg.child("filter_opponent") != NULL) {
			if(def == unitmap_->end() || !def->second.matches_filter(*cfg.child("filter_opponent"),dloc_)) {
				return false;
			}
			
			if(cfg.child("filter_opponent")->child("filter_weapon") != NULL) {
				if(!other_attack_ || !other_attack_->matches_filter(*cfg.child("filter_opponent")->child("filter_weapon"),0,true)) {
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
				if(!matches_filter(*cfg.child("filter_self")->child("filter_weapon"),0,true)) {
					return false;
				}
			}
		}
		if(cfg.child("filter_opponent") != NULL) {
			if(att == unitmap_->end() || !att->second.matches_filter(*cfg.child("filter_opponent"),aloc_)) {
				return false;
			}
			if(cfg.child("filter_opponent")->child("filter_weapon") != NULL) {
				if(!other_attack_ || !other_attack_->matches_filter(*cfg.child("filter_opponent")->child("filter_weapon"),0,true)) {
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
				if(!matches_filter(*cfg.child("filter_attacker")->child("filter_weapon"),0,true)) {
					return false;
				}
			}
		} else {
			if(cfg.child("filter_attacker")->child("filter_weapon") != NULL) {
				if(!other_attack_ || !other_attack_->matches_filter(*cfg.child("filter_attacker")->child("filter_weapon"),0,true)) {
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
				if(!matches_filter(*cfg.child("filter_defender")->child("filter_weapon"),0,true)) {
					return false;
				}
			}
		} else {
			if(cfg.child("filter_defender")->child("filter_weapon") != NULL) {
				if(!other_attack_ || !other_attack_->matches_filter(*cfg.child("filter_defender")->child("filter_weapon"),0,true)) {
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
	int index=-1;
	const config::child_list& adj_filt = cfg.get_children("filter_adjacent");
	config::child_list::const_iterator i;
	for(i = adj_filt.begin(); i != adj_filt.end(); ++i) {
		std::vector<std::string> dirs = utils::split((**i)["adjacent"]);
		if(dirs.size()==1 && dirs.front()=="") {	
		} else {
			for(std::vector<std::string>::const_iterator j = dirs.begin(); j != dirs.end(); ++j) {
				if(*j=="n") {
					index=0;
				} else if(*j=="ne") {
					index=1;
				} else if(*j=="se") {
					index=2;
				} else if(*j=="s") {
					index=3;
				} else if(*j=="sw") {
					index=4;
				} else if(*j=="nw") {
					index=5;
				} else {
					index=-1;
				}
				if(index != -1) {
					units_map::const_iterator unit = unitmap_->find(adjacent[index]);
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
	index=-1;
	const config::child_list& adj_filt_loc = cfg.get_children("filter_adjacent_location");
	for(i = adj_filt_loc.begin(); i != adj_filt_loc.end(); ++i) {
		std::vector<std::string> dirs = utils::split((**i)["adjacent"]);
		if(dirs.size()==1 && dirs.front()=="") {	
		} else {
			for(std::vector<std::string>::const_iterator j = dirs.begin(); j != dirs.end(); ++j) {
				if(*j=="n") {
					index=0;
				} else if(*j=="ne") {
					index=1;
				} else if(*j=="se") {
					index=2;
				} else if(*j=="s") {
					index=3;
				} else if(*j=="sw") {
					index=4;
				} else if(*j=="nw") {
					index=5;
				} else {
					index=-1;
				}
				if(index != -1) {
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
                              const game_data* gamedata, unit_map* unitmap, 
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


bool weapon_special_list::empty() const
{
	return cfgs.empty();
}

int weapon_special_list::highest(const std::string& key, int def) const
{
	int flat = def;
	int stack = 0;
	for(config::child_list::const_iterator i = cfgs.begin(); i != cfgs.end(); ++i) {
		if((**i)["cumulative"]=="yes") {
			stack += lexical_cast_default<int>((**i)[key]);
		} else {
			flat = maximum<int>(flat,lexical_cast_default<int>((**i)[key]));
		}
	}
	return flat + stack;
}
int weapon_special_list::lowest(const std::string& key, int def) const
{
	int flat = def;
	int stack = 0;
	for(config::child_list::const_iterator i = cfgs.begin(); i != cfgs.end(); ++i) {
		if((**i)["cumulative"]=="yes") {
			stack += lexical_cast_default<int>((**i)[key]);
		} else {
			flat = minimum<int>(flat,lexical_cast_default<int>((**i)[key]));
		}
	}
	return flat + stack;
}






