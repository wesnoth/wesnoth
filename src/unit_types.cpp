/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#include "game_config.hpp"
#include "gamestatus.hpp"
#include "language.hpp"
#include "unit_types.hpp"
#include "util.hpp"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>

//these headers are used to check for file existence
#ifdef linux
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

attack_type::attack_type(const config& cfg)
{
	name_ = cfg["name"];
	type_ = cfg["type"];
	special_ = cfg["special"];
	range_ = cfg["range"] == "long" ? LONG_RANGE : SHORT_RANGE;
	damage_ = atol(cfg["damage"].c_str());
	num_attacks_ = atol(cfg["number"].c_str());

	config::const_child_itors range = cfg.child_range("frame");
	for(; range.first != range.second; ++range.first){
		const int beg = atoi((**range.first)["begin"].c_str());
		const int end = atoi((**range.first)["end"].c_str());
		const int xoff = atoi((**range.first)["xoffset"].c_str());
		const std::string& img = (**range.first)["image"];
		frames_[UNIT_FRAME].push_back(frame(beg,end,img,xoff));
	}

	range = cfg.child_range("missile_frame");
	for(; range.first != range.second; ++range.first){
		const int beg = atoi((**range.first)["begin"].c_str());
		const int end = atoi((**range.first)["end"].c_str());
		const int xoff = atoi((**range.first)["xoffset"].c_str());
		
		const std::string& img = (**range.first)["image"];
		const std::string& img_diag = (**range.first)["image_diagonal"];
		if(img_diag.empty())
			frames_[MISSILE_FRAME].push_back(frame(beg,end,img,xoff));
		else
			frames_[MISSILE_FRAME].push_back(frame(beg,end,img,img_diag,xoff));

	}

	range = cfg.child_range("sound");
	for(; range.first != range.second; ++range.first){
		sfx sound;
		sound.time = atoi((**range.first)["time"].c_str());
		sound.on_hit = (**range.first)["sound"];
		sound.on_miss = (**range.first)["sound_miss"];
		if(sound.on_miss.empty())
			sound.on_miss = sound.on_hit;

		if(sound.on_miss == "null")
			sound.on_miss = "";

		sfx_.push_back(sound);
	}
}

const std::string& attack_type::name() const
{
	return name_;
}

const std::string& attack_type::type() const
{
	return type_;
}

const std::string& attack_type::special() const
{
	return special_;
}

attack_type::RANGE attack_type::range() const
{
	return range_;
}

int attack_type::damage() const
{
	return damage_;
}

int attack_type::num_attacks() const
{
	return num_attacks_;
}

int attack_type::get_first_frame(attack_type::FRAME_TYPE type) const
{
	if(frames_[type].empty())
		return 0;
	else
		return minimum<int>(frames_[type].front().start,0);
}

int attack_type::get_last_frame(attack_type::FRAME_TYPE type) const
{
	if(frames_[type].empty())
		return 0;
	else
		return maximum<int>(frames_[type].back().end,0);
}

const std::string* attack_type::get_frame(int milliseconds, int* xoff,
                                       attack_type::FRAME_TYPE type,
								       attack_type::FRAME_DIRECTION dir) const
{
	for(std::vector<frame>::const_iterator i = frames_[type].begin();
	    i != frames_[type].end(); ++i) {
		if(i->start > milliseconds)
			return NULL;

		if(i->start <= milliseconds && i->end > milliseconds) {
			if(xoff != NULL) {
				*xoff = i->xoffset;
			}

			if(dir == DIAGONAL && i->image_diagonal != NULL) {
				return i->image_diagonal;
			} else {
				return i->image;
			}
		}
	}

	return NULL;
}

const std::vector<attack_type::sfx>& attack_type::sound_effects() const
{
	return sfx_;
}

bool attack_type::matches_filter(const config& cfg) const
{
	const std::string& filter_range = cfg["range"];
	const std::string& filter_name = cfg["name"];
	const std::string& filter_type = cfg["type"];
	const std::string& filter_special = cfg["special"];

	if(filter_range.empty() == false) {
		if(filter_range == "short" && range() == LONG_RANGE ||
		   filter_range == "long" && range() == SHORT_RANGE) {
			return false;
		}
	}

	if(filter_name.empty() == false && filter_name != name())
		return false;

	if(filter_type.empty() == false && filter_type != type())
		return false;

	if(filter_special.empty() == false && filter_special != special())
		return false;

	return true;
}

void attack_type::apply_modification(const config& cfg)
{
	if(!matches_filter(cfg))
		return;

	const std::string& set_name = cfg["set_name"];
	const std::string& set_type = cfg["set_type"];
	const std::string& set_special = cfg["set_special"];
	const std::string& increase_damage = cfg["increase_damage"];
	const std::string& multiply_damage = cfg["multiply_damage"];
	const std::string& increase_attacks = cfg["increase_attacks"];

	if(set_name.empty() == false) {
		name_ = set_name;
	}

	if(set_type.empty() == false) {
		type_ = set_type;
	}

	if(set_special.empty() == false) {
		special_ = set_special;
	}

	if(increase_damage.empty() == false) {
		const int increase = atoi(increase_damage.c_str());
		damage_ += increase;
		if(damage_ < 1)
			damage_ = 1;
	}

	if(multiply_damage.empty() == false) {
		const double multiply = atof(increase_damage.c_str());
		if(multiply != 0.0) {
			damage_ = int(double(damage_)*multiply);
			if(damage_ < 1)
				damage_ = 1;
		}
	}

	if(increase_attacks.empty() == false) {
		const int increase = atoi(increase_attacks.c_str());
		num_attacks_ += increase;
		if(num_attacks_ < 1) {
			num_attacks_ = 1;
		}
	}
}

unit_movement_type::unit_movement_type(const config& cfg) : cfg_(cfg)
{}

const std::string& unit_movement_type::name() const
{
	return cfg_["name"];
}

int unit_movement_type::movement_cost(const gamemap& map,
                                      gamemap::TERRAIN terrain) const
{
	const std::map<gamemap::TERRAIN,int>::const_iterator i =
	                                            moveCosts_.find(terrain);
	if(i != moveCosts_.end()) {
		return i->second;
	}

	const config* movement_costs = cfg_.child("movement costs");
	if(movement_costs == NULL) {
		return 1;
	}

	const std::string& name = map.underlying_terrain_name(terrain);
	if(terrain == 'b') {
		std::cout << "underlying terrain: " << name << "\n";
	}
	const std::string& val = (*movement_costs)[name];
	int res = atoi(val.c_str());

	//don't allow 0-movement terrain
	if(res == 0) {
		res = 100;
	}

	moveCosts_.insert(std::pair<gamemap::TERRAIN,int>(terrain,res));

	return res;
}

double unit_movement_type::defense_modifier(const gamemap& map,
                                            gamemap::TERRAIN terrain) const
{
	const std::map<gamemap::TERRAIN,double>::const_iterator i =
	                                          defenseMods_.find(terrain);
	if(i != defenseMods_.end()) {
		return i->second;
	}

	const config* const defense = cfg_.child("defense");
	if(defense == NULL) {
		return 1;
	}

	const std::string& name = map.underlying_terrain_name(terrain);
	const std::string& val = (*defense)[name];

	const double res = atof(val.c_str());
	defenseMods_.insert(std::pair<gamemap::TERRAIN,double>(terrain,res));
	return res;
}

int unit_movement_type::damage_against(const attack_type& attack) const
{
	const config* const resistance = cfg_.child("resistance");
	if(resistance == NULL) {
		return 1;
	}

	const std::string& val = (*resistance)[attack.type()];
	const double resist = atof(val.c_str());
	return static_cast<int>(resist * static_cast<double>(attack.damage()));
}

const string_map& unit_movement_type::damage_table() const
{
	const config* const resistance = cfg_.child("resistance");
	if(resistance == NULL) {
		static const std::map<std::string,std::string> default_val;
		return default_val;
	} else {
		return resistance->values;
	}
}

unit_type::unit_type(const config& cfg, const movement_type_map& mv_types,
                     const std::vector<config*>& traits)
                              : cfg_(cfg), alpha_(1.0), possibleTraits_(traits)
{
	if(has_ability("heals")) {
		heals_ = game_config::healer_heals_per_turn;
		max_heals_ = game_config::heal_amount;
	} else if(has_ability("cures")) {
		heals_ = game_config::curer_heals_per_turn;
		max_heals_ = game_config::cure_amount;
	} else {
		heals_ = 0;
		max_heals_ = 0;
	}

	regenerates_ = has_ability("regenerates");
	leadership_ = has_ability("leadership");
	illuminates_ = has_ability("illuminates");
	skirmish_ = has_ability("skirmisher");
	teleport_ = has_ability("teleport");
	nightvision_ = has_ability("night vision");

	const std::string& alpha_blend = cfg_["alpha"];
	if(alpha_blend.empty() == false) {
		alpha_ = atof(alpha_blend.c_str());
	}

	const std::string& move_type = cfg_["movement_type"];
	if(move_type.empty()) {
		throw gamestatus::load_game_failed("Movement type not specified for "
		                                    "unit '" + name() + "'");
	}

	const movement_type_map::const_iterator it = mv_types.find(move_type);
	if(it == mv_types.end()) {
		throw gamestatus::load_game_failed("Undefined movement type '" +
		                                   move_type + "'");
	}

	movementType_ = &(it->second);

	//check if the images necessary for units exist
#ifdef linux
	struct stat stat_buf;
	if(game_config::path.empty() == false) {
		if(::stat((game_config::path + "/images/" +
		           cfg_["image"]).c_str(),&stat_buf) >= 0) {
			return;
		}
	}

	if(::stat(("images/" + cfg_["image"]).c_str(),&stat_buf) < 0) {
		std::cerr << "image '" << cfg_["image"] << "' does not exist!\n";
	}
#endif
}

std::string unit_type::id() const
{
	std::string n = name();
	n.erase(std::remove(n.begin(),n.end(),' '),n.end());
	return n;
}

std::string unit_type::language_name() const
{
	const std::string& lang_name = string_table[id()];
	if(lang_name.empty() == false)
		return lang_name;
	else
		return name();
}

const std::string& unit_type::name() const
{
	return cfg_["name"];
}

const std::string& unit_type::image() const
{
	return cfg_["image"];
}

const std::string& unit_type::image_fighting(attack_type::RANGE range) const
{
	static const std::string short_range("image_short");
	static const std::string long_range("image_long");

	const std::string& str = range == attack_type::LONG_RANGE ?
	                                  long_range : short_range;
	const std::string& val = cfg_[str];

	if(!val.empty())
		return val;
	else
		return image();
}

const std::string& unit_type::image_defensive(attack_type::RANGE range) const
{
	{
		static const std::string short_range("image_defensive_short");
		static const std::string long_range("image_defensive_long");

		const std::string& str = range == attack_type::LONG_RANGE ?
		                          long_range : short_range;

		const std::string& val = cfg_[str];

		if(!val.empty())
			return val;
	}

	const std::string& val = cfg_["image_defensive"];
	if(val.empty())
		return cfg_["image"];
	else
		return val;
}

const std::string& unit_type::image_profile() const
{
	const std::string& val = cfg_["profile"];
	if(val.size() == 0)
		return image();
	else
		return val;
}

const std::string& unit_type::unit_description() const
{
	static const std::string default_val("No description available");

	const std::string& lang_desc = string_table[id() + "_description"];
	if(lang_desc.empty() == false)
		return lang_desc;

	const std::string& desc = cfg_["unit_description"];
	if(desc.empty())
		return default_val;
	else
		return desc;
}

int unit_type::hitpoints() const
{
	return atoi(cfg_["hitpoints"].c_str());
}

std::vector<attack_type> unit_type::attacks() const
{
	std::vector<attack_type> res;
	for(config::const_child_itors range = cfg_.child_range("attack");
	    range.first != range.second; ++range.first) {
		res.push_back(attack_type(**range.first));
	}

	return res;
}

const unit_movement_type& unit_type::movement_type() const
{
	return *movementType_;
}

int unit_type::cost() const
{
	return atoi(cfg_["cost"].c_str());
}

int unit_type::experience_needed() const
{
	return atoi(cfg_["experience"].c_str());
}

std::vector<std::string> unit_type::advances_to() const
{
	const std::string& val = cfg_["advanceto"];
	if(val == "null" || val == "")
		return std::vector<std::string>();
	else
		return config::split(val);
}

const std::string& unit_type::usage() const
{
	return cfg_["usage"];
}

int unit_type::level() const
{
	return atoi(cfg_["level"].c_str());
}

int unit_type::movement() const
{
	return atoi(cfg_["movement"].c_str());
}

unit_type::ALIGNMENT unit_type::alignment() const
{
	const std::string& align = cfg_["alignment"];
	if(align == "lawful")
		return LAWFUL;
	else if(align == "chaotic")
		return CHAOTIC;
	else
		return NEUTRAL;
}

const std::string& unit_type::alignment_description(unit_type::ALIGNMENT align)
{
	static const std::string aligns[] = { "lawful", "neutral", "chaotic" };
	const string_map::const_iterator i = string_table.find(aligns[align]);
	if(i != string_table.end())
		return i->second;
	else
		return aligns[align];
}

double unit_type::alpha() const
{
	return alpha_;
}

const std::string& unit_type::ability() const
{
	return cfg_["ability"];
}

int unit_type::max_unit_healing() const
{
	return max_heals_;
}

int unit_type::heals() const
{
	return heals_;
}

bool unit_type::regenerates() const
{
	return regenerates_;
}

bool unit_type::is_leader() const
{
	return leadership_;
}

bool unit_type::illuminates() const
{
	return illuminates_;
}

bool unit_type::is_skirmisher() const
{
	return skirmish_;
}

bool unit_type::teleports() const
{
	return teleport_;
}

bool unit_type::nightvision() const
{
	return nightvision_;
}

bool unit_type::has_ability(const std::string& ability) const
{
	return config::has_value(this->ability(),ability);
}

const std::vector<config*>& unit_type::possible_traits() const
{
	return possibleTraits_;
}

game_data::game_data(const config& cfg)
{
	static const std::vector<config*> dummy_traits;
	const config::child_map::const_iterator traits = cfg.children.find("trait");
	const std::vector<config*>& unit_traits = traits == cfg.children.end() ?
	                                  dummy_traits : traits->second;

	for(config::const_child_itors i = cfg.child_range("movetype");
	    i.first != i.second; ++i.first) {
		const unit_movement_type move_type(**i.first);
		movement_types.insert(
				std::pair<std::string,unit_movement_type>(move_type.name(),
						                                  move_type));
	}

	for(config::const_child_itors j = cfg.child_range("unit");
	    j.first != j.second; ++j.first) {
		const unit_type u_type(**j.first,movement_types,unit_traits);
		unit_types.insert(
				std::pair<std::string,unit_type>(u_type.name(),u_type));
	}
}
