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

#include "global.hpp"

#include "game_config.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "unit_types.hpp"
#include "util.hpp"
#include "wassert.hpp"
#include "serialization/string_utils.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>

unit_animation::frame::frame(const config& cfg)
{
	xoffset = atoi(cfg["xoffset"].c_str()); 
	image = cfg["image"];
	image_diagonal = cfg["image_diagonal"];
	halo = cfg["halo"];
	halo_x = atoi(cfg["halo_x"].c_str());
	halo_y = atoi(cfg["halo_y"].c_str());
}

unit_animation::unit_animation()
{}

unit_animation::unit_animation(const config& cfg) 
{
	config::const_child_itors range = cfg.child_range("frame");
	
	int last_end = INT_MIN;
	for(; range.first != range.second; ++range.first) {
		unit_frames_.add_frame(atoi((**range.first)["begin"].c_str()), frame(**range.first));
		last_end = maximum<int>(atoi((**range.first)["end"].c_str()), last_end);
	}
	//unit_frames_.set_animation_end(last_end);
	unit_frames_.add_frame(last_end);

	last_end = INT_MIN;
	range = cfg.child_range("missile_frame");
	for(; range.first != range.second; ++range.first) {
		missile_frames_.add_frame(atoi((**range.first)["begin"].c_str()), frame(**range.first));
		last_end = maximum<int>(atoi((**range.first)["end"].c_str()), last_end);
	}
	missile_frames_.add_frame(last_end);

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

int unit_animation::get_first_frame_time(unit_animation::FRAME_TYPE type) const
{
	if(type == UNIT_FRAME) {
		return unit_frames_.get_first_frame_time();
	} else {
		return missile_frames_.get_first_frame_time();
	}
}

int unit_animation::get_last_frame_time(unit_animation::FRAME_TYPE type) const
{
	if(type == UNIT_FRAME) {
		return unit_frames_.get_last_frame_time();
	} else {
		return missile_frames_.get_last_frame_time();
	}
}

void unit_animation::start_animation(int start_frame, FRAME_TYPE type, int acceleration)
{
	if (type == UNIT_FRAME) {
		unit_frames_.start_animation(start_frame, 1, acceleration);
	} else {
		missile_frames_.start_animation(start_frame, 1, acceleration);
	}
}

void unit_animation::update_current_frames()
{
	unit_frames_.update_current_frame();
	missile_frames_.update_current_frame();
}

bool unit_animation::animation_finished() const
{
	return unit_frames_.animation_finished() && missile_frames_.animation_finished();
}

const unit_animation::frame& unit_animation::get_current_frame(FRAME_TYPE type) const
{
	if(type == UNIT_FRAME) {
		return unit_frames_.get_current_frame();
	} else {
		return missile_frames_.get_current_frame();
	}
}

int unit_animation::get_animation_time() const
{
	return unit_frames_.get_animation_time();
}

const std::vector<unit_animation::sfx>& unit_animation::sound_effects() const
{
	return sfx_;
}

attack_type::attack_type(const config& cfg)
{
	const config::child_list& animations = cfg.get_children("animation");
	for(config::child_list::const_iterator an = animations.begin(); an != animations.end(); ++an) {
		const std::string& dir = (**an)["direction"];
		if(dir == "") {
			animation_.push_back(unit_animation(**an));
		} else {
			const std::vector<std::string>& directions = utils::split(dir);
			for(std::vector<std::string>::const_iterator i = directions.begin(); i != directions.end(); ++i) {
				const gamemap::location::DIRECTION d = gamemap::location::parse_direction(*i);
				if(d != gamemap::location::NDIRECTIONS) {
					direction_animation_[d].push_back(unit_animation(**an));
				}
			}
		}
	}

	if(animation_.empty()) {
		animation_.push_back(unit_animation(cfg));
	}

	name_ = cfg["name"];
	type_ = cfg["type"];
	special_ = cfg["special"];
	backstab_ = special_ == "backstab";
	slow_ = special_ == "slow";
	icon_ = cfg["icon"];
	if(icon_.empty())
		icon_ = "attacks/" + name_ + ".png";

	range_ = cfg["range"] == "long" ? LONG_RANGE : SHORT_RANGE;
	damage_ = atol(cfg["damage"].c_str());
	num_attacks_ = atol(cfg["number"].c_str());

	attack_weight_ = atof(cfg["attack_weight"].c_str());
	defense_weight_ = atof(cfg["defense_weight"].c_str());
	if ( ! attack_weight_ )
	  attack_weight_ = 1.0;
	if ( ! defense_weight_ )
	  defense_weight_ = 1.0;
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

const std::string& attack_type::icon() const
{
	return icon_;
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

double attack_type::attack_weight() const
{
	return attack_weight_;
}

double attack_type::defense_weight() const
{
	return defense_weight_;
}

bool attack_type::backstab() const
{
	return backstab_;
}

bool attack_type::slow() const
{
	return slow_;
}

const unit_animation& attack_type::animation(const gamemap::location::DIRECTION dir) const
{
	const std::vector<unit_animation>* animation = &animation_;
	if(dir < sizeof(direction_animation_)/sizeof(*direction_animation_) &&
	   direction_animation_[dir].empty() == false) {
		animation = &direction_animation_[dir];
	}

	wassert(animation->empty() == false);
	return (*animation)[rand()%animation->size()];
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

bool attack_type::apply_modification(const config& cfg, std::string* description)
{
	if(!matches_filter(cfg))
		return false;

	const std::string& set_name = cfg["set_name"];
	const std::string& set_type = cfg["set_type"];
	const std::string& set_special = cfg["set_special"];
	const std::string& increase_damage = cfg["increase_damage"];
	const std::string& increase_attacks = cfg["increase_attacks"];

	std::stringstream desc;

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
		int increase = 0;
		
		if(increase_damage[increase_damage.size()-1] == '%') {
			const std::string inc(increase_damage.begin(),increase_damage.end()-1);
			const int percent = atoi(inc.c_str());
			increase = (damage_*percent)/100;
		} else {
			increase = atoi(increase_damage.c_str());
		}

		damage_ += increase;
		if(damage_ < 1) {
			damage_ = 1;
		}

		if(description != NULL) {
			desc << (increase_damage[0] == '-' ? "" : "+") << increase_damage << _("Dmg");
		}
	}

	if(increase_attacks.empty() == false) {
		const int increase = atoi(increase_attacks.c_str());
		num_attacks_ += increase;
		if(num_attacks_ < 1) {
			num_attacks_ = 1;
		}

		if(description != NULL) {
			desc << (increase > 0 ? "+" : "") << increase << _("strikes");
		}
	}

	if(description != NULL) {
		*description = desc.str();
	}

	return true;
}

unit_movement_type::unit_movement_type(const config& cfg, const unit_movement_type* parent)
             : cfg_(cfg), parent_(parent)
{}

const std::string& unit_movement_type::name() const
{
	const std::string& res = cfg_["name"];
	if(res == "" && parent_ != NULL)
		return parent_->name();
	else
		return res;
}

int unit_movement_type::movement_cost(const gamemap& map,gamemap::TERRAIN terrain,int recurse_count) const
{
	const int impassable = 10000000;

	const std::map<gamemap::TERRAIN,int>::const_iterator i = moveCosts_.find(terrain);
	if(i != moveCosts_.end()) {
		return i->second;
	}

	//if this is an alias, then select the best of all underlying terrains
	const std::string& underlying = map.underlying_terrain(terrain);
	if(underlying.size() != 1 || underlying[0] != terrain) {
		if(recurse_count >= 100) {
			return impassable;
		}

		int min_value = impassable;
		for(std::string::const_iterator i = underlying.begin(); i != underlying.end(); ++i) {
			const int value = movement_cost(map,*i,recurse_count+1);
			if(value < min_value) {
				min_value = value;
			}
		}

		moveCosts_.insert(std::pair<gamemap::TERRAIN,int>(terrain,min_value));

		return min_value;
	}

	const config* movement_costs = cfg_.child("movement costs");

	int res = -1;

	if(movement_costs != NULL) {
		if(underlying.size() != 1) {
			lg::err(lg::config) << "terrain '" << terrain << "' has " << underlying.size() << " underlying names - 0 expected\n";
			return impassable;
		}

		const std::string& id = map.get_terrain_info(underlying[0]).id();

		const std::string& val = (*movement_costs)[id];

		if(val != "") {
			res = atoi(val.c_str());
		}
	}

	if(res <= 0 && parent_ != NULL) {
		res = parent_->movement_cost(map,terrain);
	}

	if(res <= 0) {
		res = impassable;
	}

	moveCosts_.insert(std::pair<gamemap::TERRAIN,int>(terrain,res));

	return res;
}

int unit_movement_type::defense_modifier(const gamemap& map,gamemap::TERRAIN terrain, int recurse_count) const
{
	const std::map<gamemap::TERRAIN,int>::const_iterator i = defenseMods_.find(terrain);
	if(i != defenseMods_.end()) {
		return i->second;
	}

	//if this is an alias, then select the best of all underlying terrains
	const std::string& underlying = map.underlying_terrain(terrain);
	if(underlying.size() != 1 || underlying[0] != terrain) {
		if(recurse_count >= 100) {
			return 100;
		}

		int min_value = 100;
		for(std::string::const_iterator i = underlying.begin(); i != underlying.end(); ++i) {
			const int value = defense_modifier(map,*i,recurse_count+1);
			if(value < min_value) {
				min_value = value;
			}
		}

		defenseMods_.insert(std::pair<gamemap::TERRAIN,int>(terrain,min_value));

		return min_value;
	}

	int res = -1;

	const config* const defense = cfg_.child("defense");

	if(defense != NULL) {
		if(underlying.size() != 1) {
			lg::err(lg::config) << "terrain '" << terrain << "' has " << underlying.size() << " underlying names - 0 expected\n";
			return 100;
		}

		const std::string& id = map.get_terrain_info(underlying[0]).id();
		const std::string& val = (*defense)[id];

		if(val != "") {
			res = atoi(val.c_str());
		}
	}

	if(res <= 0 && parent_ != NULL) {
		res = parent_->defense_modifier(map,terrain);
	}

	if(res <= 0) {
		res = 50;
	}

	defenseMods_.insert(std::pair<gamemap::TERRAIN,int>(terrain,res));

	return res;
}

int unit_movement_type::damage_against(const attack_type& attack) const
{
	return resistance_against(attack);
}

int unit_movement_type::resistance_against(const attack_type& attack) const
{
	bool result_found = false;
	int res = 0;

	const config* const resistance = cfg_.child("resistance");
	if(resistance != NULL) {
		const std::string& val = (*resistance)[attack.type()];
		if(val != "") {
			res = atoi(val.c_str());
			result_found = true;
		}
	}

	if(!result_found && parent_ != NULL) {
		res = parent_->resistance_against(attack);
	}

	return res;
}

string_map unit_movement_type::damage_table() const
{
	string_map res;
	if(parent_ != NULL)
		res = parent_->damage_table();

	const config* const resistance = cfg_.child("resistance");
	if(resistance != NULL) {
		for(string_map::const_iterator i = resistance->values.begin(); i != resistance->values.end(); ++i) {
			res[i->first] = i->second;
		}
	}

	return res;
}

bool unit_movement_type::is_flying() const
{
	const std::string& flies = cfg_["flies"];
	if(flies == "" && parent_ != NULL)
		return parent_->is_flying();

	return flies == "true";
}

void unit_movement_type::set_parent(const unit_movement_type* parent)
{
	parent_ = parent;
}

unit_type::unit_type(const unit_type& o)
    : variations_(o.variations_), cfg_(o.cfg_), race_(o.race_),
      alpha_(o.alpha_), abilities_(o.abilities_),
      max_heals_(o.max_heals_), heals_(o.heals_), regenerates_(o.regenerates_),
      leadership_(o.leadership_), illuminates_(o.illuminates_),
      skirmish_(o.skirmish_), teleport_(o.teleport_),
      nightvision_(o.nightvision_), steadfast_(o.steadfast_),
      can_advance_(o.can_advance_), alignment_(o.alignment_),
      movementType_(o.movementType_), possibleTraits_(o.possibleTraits_),
      genders_(o.genders_), defensive_animations_(o.defensive_animations_),
      teleport_animations_(o.teleport_animations_), death_animations_(o.death_animations_)
{
	gender_types_[0] = o.gender_types_[0] != NULL ? new unit_type(*o.gender_types_[0]) : NULL;
	gender_types_[1] = o.gender_types_[1] != NULL ? new unit_type(*o.gender_types_[1]) : NULL;

	for(variations_map::const_iterator i = o.variations_.begin(); i != o.variations_.end(); ++i) {
		variations_[i->first] = new unit_type(*i->second);
	}
}

unit_type::unit_type(const config& cfg, const movement_type_map& mv_types,
                     const race_map& races, const std::vector<config*>& traits)
	: cfg_(cfg), alpha_(ftofxp(1.0)), movementType_(cfg), possibleTraits_(traits)
{
	const config::child_list& variations = cfg.get_children("variation");
	for(config::child_list::const_iterator var = variations.begin(); var != variations.end(); ++var) {
		variations_.insert(std::pair<std::string,unit_type*>((**var)["variation_name"],new unit_type(**var,mv_types,races,traits)));
	}

	gender_types_[0] = NULL;
	gender_types_[1] = NULL;

	const config* const male_cfg = cfg.child("male");
	if(male_cfg != NULL) {
		gender_types_[unit_race::MALE] = new unit_type(*male_cfg,mv_types,races,traits);
	}

	const config* const female_cfg = cfg.child("female");
	if(female_cfg != NULL) {
		gender_types_[unit_race::FEMALE] = new unit_type(*female_cfg,mv_types,races,traits);
	}

	const std::vector<std::string> genders = utils::split(cfg["gender"]);
	for(std::vector<std::string>::const_iterator i = genders.begin();
	    i != genders.end(); ++i) {
		if(*i == "male") {
			genders_.push_back(unit_race::MALE);
		} else if(*i == "female") {
			genders_.push_back(unit_race::FEMALE);
		}
	}

	if(genders_.empty()) {
		genders_.push_back(unit_race::MALE);
	}

	const race_map::const_iterator race_it = races.find(cfg["race"]);
	if(race_it != races.end()) {
		race_ = &race_it->second;
		if(race_ != NULL) {
			if(race_->uses_global_traits() == false) {
				possibleTraits_.clear();
			}

			const config::child_list& traits = race_->additional_traits();
			possibleTraits_.insert(possibleTraits_.end(),traits.begin(),traits.end());
		}
	} else {
		static const unit_race dummy_race;
		race_ = &dummy_race;
	}

	//insert any traits that are just for this unit type
	const config::child_list& unit_traits = cfg.get_children("trait");
	possibleTraits_.insert(possibleTraits_.end(),unit_traits.begin(),unit_traits.end());

	abilities_ = utils::split(cfg_["ability"]);

	//if the string was empty, split will give us one empty string in the list,
	//remove it.
	if(!abilities_.empty() && abilities_.back() == "")
		abilities_.pop_back();

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
	steadfast_ = has_ability("steadfast");
	
	const std::string& align = cfg_["alignment"];
	if(align == "lawful")
		alignment_ = LAWFUL;
	else if(align == "chaotic")
		alignment_ = CHAOTIC;
	else if(align == "neutral")
		alignment_ = NEUTRAL;
	else {
		lg::err(lg::config) << "Invalid alignment found for " << name() << ": '" << align << "'\n";
		alignment_ = NEUTRAL;
	}

	const std::string& alpha_blend = cfg_["alpha"];
	if(alpha_blend.empty() == false) {
		alpha_ = ftofxp(atof(alpha_blend.c_str()));
	}

	const std::string& move_type = cfg_["movement_type"];

	const movement_type_map::const_iterator it = mv_types.find(move_type);

	if(it != mv_types.end()) {
		movementType_.set_parent(&(it->second));
	}

	can_advance_ = advances_to().empty() == false;

	const config::child_list& defends = cfg_.get_children("defend");
	for(config::child_list::const_iterator d = defends.begin(); d != defends.end(); ++d) {
		defensive_animations_.push_back(defensive_animation(**d));
	}
	const config::child_list& teleports = cfg_.get_children("teleport_anim");
	for(config::child_list::const_iterator t = teleports.begin(); t != teleports.end(); ++t) {
		teleport_animations_.push_back(unit_animation(**t));
	}

	const config::child_list& deaths = cfg_.get_children("death");
	for(config::child_list::const_iterator death = deaths.begin(); death != deaths.end(); ++death) {
		death_animations_.push_back(death_animation(**death));
	}
}

unit_type::~unit_type()
{
	delete gender_types_[unit_race::MALE];
	delete gender_types_[unit_race::FEMALE];

	for(variations_map::iterator i = variations_.begin(); i != variations_.end(); ++i) {
		delete i->second;
	}
}

const unit_type& unit_type::get_gender_unit_type(unit_race::GENDER gender) const
{
	const size_t i = gender;
	if(i < sizeof(gender_types_)/sizeof(*gender_types_) && gender_types_[i] != NULL) {
		return *gender_types_[i];
	}

	return *this;
}

const unit_type& unit_type::get_variation(const std::string& name) const
{
	const variations_map::const_iterator i = variations_.find(name);
	if(i != variations_.end()) {
		return *i->second;
	} else {
		return *this;
	}
}

int unit_type::num_traits() const { return race_->num_traits(); }

std::string unit_type::generate_description() const
{
	return race_->generate_name(cfg_["gender"] == "female" ? unit_race::FEMALE : unit_race::MALE);
}

const std::string& unit_type::id() const
{
	if(id_.empty()) {
		id_ = cfg_["id"];

		if(id_.empty()) {
			// this code is only for compatibility with old unit defs and savefiles
			id_ = cfg_["name"];
		}

		id_.erase(std::remove(id_.begin(),id_.end(),' '),id_.end());
	}

	return id_;
}

std::string unit_type::language_name() const
{
	return cfg_["name"];
}

const std::string& unit_type::name() const
{
	return cfg_["id"];
}

const std::string& unit_type::image() const
{
	return cfg_["image"];
}

const std::string& unit_type::image_halo() const
{
	return cfg_["halo"];
}

const std::string& unit_type::image_moving() const
{
	const std::string& res = cfg_["image_moving"];
	if(res.empty()) {
		return image();
	} else {
		return res;
	}
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

const std::string& unit_type::image_leading() const
{
	const std::string& val = cfg_["image_leading"];
	if(val.empty()) {
		return image();
	} else {
		return val;
	}
}

const std::string& unit_type::image_healing() const
{
	const std::string& val = cfg_["image_healing"];
	if(val.empty()) {
		return image();
	} else {
		return val;
	}
}

const std::string& unit_type::image_halo_healing() const
{
	return cfg_["image_halo_healing"];
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

	const std::string& desc = cfg_["unit_description"];
	if(desc.empty())
		return default_val;
	else
		return desc;
}

const std::string& unit_type::get_hit_sound() const
{
	return cfg_["get_hit_sound"];
}

const std::string& unit_type::die_sound() const
{
	return cfg_["die_sound"];
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
	return movementType_;
}

int unit_type::cost() const
{
	return atoi(cfg_["cost"].c_str());
}

namespace {
	int experience_modifier = 100;
}

unit_type::experience_accelerator::experience_accelerator(int modifier) : old_value_(experience_modifier)
{
	experience_modifier = (experience_modifier*modifier)/100;
}

unit_type::experience_accelerator::~experience_accelerator()
{
	experience_modifier = old_value_;
}

int unit_type::experience_needed() const
{
	return (lexical_cast_default<int>(cfg_["experience"],500)*experience_modifier)/100;
}

std::vector<std::string> unit_type::advances_to() const
{
	const std::string& val = cfg_["advanceto"];
	if(val == "null" || val == "")
		return std::vector<std::string>();
	else
		return utils::split(val);
}

const config::child_list& unit_type::modification_advancements() const
{
	return cfg_.get_children("advancement");
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
	return alignment_;
}

const char* unit_type::alignment_description(unit_type::ALIGNMENT align)
{
	static const char* aligns[] = { N_("lawful"), N_("neutral"), N_("chaotic") };
	return (gettext(aligns[align]));
}

const char* unit_type::alignment_id(unit_type::ALIGNMENT align)
{
	static const char* aligns[] = { "lawful", "neutral", "chaotic" };
	return (aligns[align]);
}

fixed_t unit_type::alpha() const
{
	return alpha_;
}

const std::vector<std::string>& unit_type::abilities() const
{
	return abilities_;
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

bool unit_type::steadfast() const
{
	return steadfast_;
}

bool unit_type::not_living() const
{
	return race_->not_living();
}

bool unit_type::can_advance() const
{
	return can_advance_;
}

bool unit_type::has_zoc() const
{
	return level() > 0;
}

bool unit_type::has_ability(const std::string& ability) const
{
	return std::find(abilities_.begin(),abilities_.end(),ability) != abilities_.end();
}

const std::vector<config*>& unit_type::possible_traits() const
{
	return possibleTraits_;
}

const std::vector<unit_race::GENDER>& unit_type::genders() const
{
	return genders_;
}

const std::string& unit_type::race() const
{
	if(race_ == NULL) {
		static const std::string empty_string;
		return empty_string;
	}

	return race_->name();
}

unit_type::defensive_animation::defensive_animation(const config& cfg) : hits(HIT_OR_MISS), range(SHORT_OR_LONG), animation(cfg)
{
	const std::string& hits_str = cfg["hits"];
	if(hits_str.empty() == false) {
		hits = (hits_str == "yes") ? HIT : MISS;
	}

	const std::string& range_str = cfg["range"];
	if(range_str.empty() == false) {
		range = (range_str == "short") ? SHORT : LONG;
	}
}

bool unit_type::defensive_animation::matches(bool h, attack_type::RANGE r) const
{
	if(hits == HIT && h == false || hits == MISS && h == true || range == SHORT && r == attack_type::LONG_RANGE || range == LONG && r == attack_type::SHORT_RANGE) {
		return false;
	} else {
		return true;
	}
}

unit_type::death_animation::death_animation(const config& cfg)
: damage_type(utils::split(cfg["damage_type"])), special(utils::split(cfg["attack_special"])), animation(cfg)
{
}

bool unit_type::death_animation::matches(const attack_type* attack) const
{
	if(attack == NULL) {
		return true;
	}

	if(damage_type.empty() == false && std::find(damage_type.begin(),damage_type.end(),attack->type()) == damage_type.end()) {
		return false;
	}

	if(special.empty() == false && std::find(special.begin(),special.end(),attack->special()) == special.end()) {
		return false;
	}

	return true;
}

const unit_animation* unit_type::defend_animation(bool hits, attack_type::RANGE range) const
{
	//select one of the matching animations at random
	const unit_animation* res = NULL;
	std::vector<const unit_animation*> options;
	for(std::vector<defensive_animation>::const_iterator i = defensive_animations_.begin(); i != defensive_animations_.end(); ++i) {
		if(i->matches(hits,range)) {
			if(res != NULL) {
				options.push_back(res);
			}

			res = &i->animation;
		}
	}

	if(options.empty()) {
		return res;
	} else {
		options.push_back(res);
		return options[rand()%options.size()];
	}
}

const unit_animation* unit_type::teleport_animation( ) const
{
	if (teleport_animations_.empty()) return NULL;
	return &teleport_animations_[rand() % teleport_animations_.size()];
}

const unit_animation* unit_type::die_animation(const attack_type* attack) const
{
	if(death_animations_.empty()) {
		return NULL;
	}

	if(attack == NULL) {
		return &death_animations_[rand()%death_animations_.size()].animation;
	}

	const unit_animation* res = NULL;
	std::vector<const unit_animation*> options;
	for(std::vector<death_animation>::const_iterator i = death_animations_.begin(); i != death_animations_.end(); ++i) {
		if(i->matches(attack)) {
			if(res != NULL) {
				options.push_back(res);
			}

			res = &i->animation;
		}
	}

	if(options.empty()) {
		return res;
	} else {
		options.push_back(res);
		return options[rand()%options.size()];
	}
}

game_data::game_data()
{}

game_data::game_data(const config& cfg)
{
	set_config(cfg);
}

void game_data::set_config(const config& cfg)
{
	static const std::vector<config*> dummy_traits;
	
	const config::child_list& unit_traits = cfg.get_children("trait");

	for(config::const_child_itors i = cfg.child_range("movetype");
	    i.first != i.second; ++i.first) {
		const unit_movement_type move_type(**i.first);
		movement_types.insert(
				std::pair<std::string,unit_movement_type>(move_type.name(),
						                                  move_type));
	}

	for(config::const_child_itors r = cfg.child_range("race");
	    r.first != r.second; ++r.first) {
		const unit_race race(**r.first);
		races.insert(std::pair<std::string,unit_race>(race.name(),race));
	}

	for(config::const_child_itors j = cfg.child_range("unit");
	    j.first != j.second; ++j.first) {
		const unit_type u_type(**j.first,movement_types,races,unit_traits);
		unit_types.insert(std::pair<std::string,unit_type>(u_type.name(),u_type));
	}
}

void game_data::clear()
{
	movement_types.clear();
	unit_types.clear();
	races.clear();
}
