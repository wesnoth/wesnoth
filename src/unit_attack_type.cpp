/*
   Copyright (C) 2003 - 2015 by David White <dave@whitevine.net>
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
 *  @file
 *  Handle unit-type specific attributes, animations, advancement.
 */

#include "global.hpp"

#include "unit_attack_type.hpp"

#include "log.hpp"
#include "serialization/string_utils.hpp"
#include "gettext.hpp"
#include <boost/foreach.hpp>

static lg::log_domain log_config("config");
#define ERR_CF LOG_STREAM(err, log_config)
#define WRN_CF LOG_STREAM(warn, log_config)
#define LOG_CONFIG LOG_STREAM(info, log_config)
#define DBG_CF LOG_STREAM(debug, log_config)

static lg::log_domain log_unit("unit");
#define DBG_UT LOG_STREAM(debug, log_unit)
#define ERR_UT LOG_STREAM(err, log_unit)

attack_type::attack_type(const config& cfg) :
	self_loc_(),
	other_loc_(),
	is_attacker_(false),
	other_attack_(NULL),
	description_(cfg["description"].t_str()),
	id_(cfg["name"]),
	type_(cfg["type"]),
	icon_(cfg["icon"]),
	range_(cfg["range"]),
	min_range_(cfg["min_range"].to_int(1)),
	max_range_(cfg["max_range"].to_int(1)),
	damage_(cfg["damage"]),
	num_attacks_(cfg["number"]),
	attack_weight_(cfg["attack_weight"].to_double(1.0)),
	defense_weight_(cfg["defense_weight"].to_double(1.0)),
	accuracy_(cfg["accuracy"]),
	movement_used_(cfg["movement_used"].to_int(100000)),
	parry_(cfg["parry"]),
	specials_(cfg.child_or_empty("specials"))
{
	if (description_.empty())
		description_ = translation::egettext(id_.c_str());

	if(icon_.empty()){
		if (id_ != "")
			icon_ = "attacks/" + id_ + ".png";
		else
			icon_ = "attacks/blank-attack.png";
	}
}

attack_type::~attack_type()
{
}

std::string attack_type::accuracy_parry_description() const
{
	if(accuracy_ == 0 && parry_ == 0) {
		return "";
	}

	std::ostringstream s;
	s << utils::signed_percent(accuracy_);

	if(parry_ != 0) {
		s << "/" << utils::signed_percent(parry_);
	}

	return s.str();
}

/**
 * Returns whether or not *this matches the given @a filter, ignoring the
 * complexities introduced by [and], [or], and [not].
 */
static bool matches_simple_filter(const attack_type & attack, const config & filter)
{
	const std::vector<std::string>& filter_range = utils::split(filter["range"]);
	const std::string& filter_damage = filter["damage"];
	const std::vector<std::string> filter_name = utils::split(filter["name"]);
	const std::vector<std::string> filter_type = utils::split(filter["type"]);
	const std::string filter_special = filter["special"];

	if ( !filter_range.empty() && std::find(filter_range.begin(), filter_range.end(), attack.range()) == filter_range.end() )
		return false;

	if ( !filter_damage.empty() && !in_ranges(attack.damage(), utils::parse_ranges(filter_damage)) )
		return false;

	if ( !filter_name.empty() && std::find(filter_name.begin(), filter_name.end(), attack.id()) == filter_name.end() )
		return false;

	if ( !filter_type.empty() && std::find(filter_type.begin(), filter_type.end(), attack.type()) == filter_type.end() )
		return false;

	if ( !filter_special.empty() && !attack.get_special_bool(filter_special, true) )
		return false;

	// Passed all tests.
	return true;
}

/**
 * Returns whether or not *this matches the given @a filter.
 */
bool attack_type::matches_filter(const config& filter) const
{
	// Handle the basic filter.
	bool matches = matches_simple_filter(*this, filter);

	// Handle [and], [or], and [not] with in-order precedence
	BOOST_FOREACH( const config::any_child &condition, filter.all_children_range() )
	{
		// Handle [and]
		if ( condition.key == "and" )
			matches = matches && matches_filter(condition.cfg);

		// Handle [or]
		else if ( condition.key == "or" )
			matches = matches || matches_filter(condition.cfg);

		// Handle [not]
		else if ( condition.key == "not" )
			matches = matches && !matches_filter(condition.cfg);
	}

	return matches;
}

namespace {
	void add_and(std::stringstream &ss) {
		if(ss.tellp() > 0)
			ss << t_string(N_(" and "), "wesnoth");
	}
}

/**
 * Modifies *this using the specifications in @a cfg, but only if *this matches
 * @a cfg viewed as a filter.
 *
 * If *description is provided, it will be set to a (translated) description
 * of the modification(s) applied (currently only changes to the number of
 * strikes, damage, accuracy, and parry are included in this description).
 *
 * @returns whether or not @c this matched the @a cfg as a filter.
 */
bool attack_type::apply_modification(const config& cfg,std::string* description)
{
	if( !matches_filter(cfg) )
		return false;

	const std::string& set_name = cfg["set_name"];
	const t_string& set_desc = cfg["set_description"];
	const std::string& set_type = cfg["set_type"];
	const std::string& set_icon = cfg["set_icon"];
	const std::string& del_specials = cfg["remove_specials"];
	const config &set_specials = cfg.child("set_specials");
	const std::string& increase_damage = cfg["increase_damage"];
	const std::string& increase_attacks = cfg["increase_attacks"];
	const std::string& set_attack_weight = cfg["attack_weight"];
	const std::string& set_defense_weight = cfg["defense_weight"];
	const std::string& increase_accuracy = cfg["increase_accuracy"];
	const std::string& increase_parry = cfg["increase_parry"];

	std::stringstream desc;

	if(set_name.empty() == false) {
		id_ = set_name;
	}

	if(set_desc.empty() == false) {
		description_ = set_desc;
	}

	if(set_type.empty() == false) {
		type_ = set_type;
	}

	if(set_icon.empty() == false) {
		icon_ = set_icon;
	}

	if(del_specials.empty() == false) {
		const std::vector<std::string>& dsl = utils::split(del_specials);
		config new_specials;
		BOOST_FOREACH(const config::any_child &vp, specials_.all_children_range()) {
			std::vector<std::string>::const_iterator found_id =
				std::find(dsl.begin(), dsl.end(), vp.cfg["id"].str());
			if (found_id == dsl.end()) {
				new_specials.add_child(vp.key, vp.cfg);
			}
		}
		specials_ = new_specials;
	}

	if (set_specials) {
		const std::string &mode = set_specials["mode"];
		if (mode != "append") {
			specials_.clear();
		}
		BOOST_FOREACH(const config::any_child &value, set_specials.all_children_range()) {
			specials_.add_child(value.key, value.cfg);
		}
	}

	if(increase_damage.empty() == false) {
		damage_ = utils::apply_modifier(damage_, increase_damage, 0);
		if (damage_ < 0) {
			damage_ = 0;
		}

		if(description != NULL) {
			add_and(desc);
			int inc_damage = lexical_cast<int>(increase_damage);
			desc << utils::print_modifier(increase_damage) << " "
				 << _n("damage","damage", inc_damage);
		}
	}

	if(increase_attacks.empty() == false) {
		num_attacks_ = utils::apply_modifier(num_attacks_, increase_attacks, 1);

		if(description != NULL) {
			add_and(desc);
			int inc_attacks = lexical_cast<int>(increase_attacks);
			desc << utils::print_modifier(increase_attacks) << " "
				 << _n("strike", "strikes", inc_attacks);
		}
	}

	if(increase_accuracy.empty() == false) {
		accuracy_ = utils::apply_modifier(accuracy_, increase_accuracy, 1);

		if(description != NULL) {
			add_and(desc);
			int inc_acc = lexical_cast<int>(increase_accuracy);
			// Help xgettext with a directive to recognize the string as a non C printf-like string
			// xgettext:no-c-format
			desc << utils::signed_value(inc_acc) << _("% accuracy");
		}
	}

	if(increase_parry.empty() == false) {
		parry_ = utils::apply_modifier(parry_, increase_parry, 1);

		if(description != NULL) {
			add_and(desc);
			int inc_parry = lexical_cast<int>(increase_parry);
			// xgettext:no-c-format
			desc << utils::signed_value(inc_parry) << _("% parry");
		}
	}

	if(set_attack_weight.empty() == false) {
		attack_weight_ = lexical_cast_default<double>(set_attack_weight,1.0);
	}

	if(set_defense_weight.empty() == false) {
		defense_weight_ = lexical_cast_default<double>(set_defense_weight,1.0);
	}

	if(description != NULL) {
		*description = desc.str();
	}

	return true;
}

/**
 * Trimmed down version of apply_modification(), with no modifications actually
 * made. This can be used to get a description of the modification(s) specified
 * by @a cfg (if *this matches cfg as a filter).
 *
 * If *description is provided, it will be set to a (translated) description
 * of the modification(s) that would be applied to the number of strikes
 * and damage.
 *
 * @returns whether or not @c this matched the @a cfg as a filter.
 */
bool attack_type::describe_modification(const config& cfg,std::string* description)
{
	if( !matches_filter(cfg) )
		return false;

	// Did the caller want the description?
	if(description != NULL) {
		const std::string& increase_damage = cfg["increase_damage"];
		const std::string& increase_attacks = cfg["increase_attacks"];

		std::stringstream desc;

		if(increase_damage.empty() == false) {
			add_and(desc);
			int inc_damage = lexical_cast<int>(increase_damage);
			desc << utils::print_modifier(increase_damage) << " "
				 << _n("damage","damage", inc_damage);
		}

		if(increase_attacks.empty() == false) {
			add_and(desc);
			int inc_attacks = lexical_cast<int>(increase_attacks);
			desc << utils::print_modifier(increase_attacks) << " "
				 << _n("strike", "strikes", inc_attacks);
		}

		*description = desc.str();
	}

	return true;
}

void attack_type::write(config& cfg) const
{
	cfg["description"] = description_;
	cfg["name"] = id_;
	cfg["type"] = type_;
	cfg["icon"] = icon_;
	cfg["range"] = range_;
	cfg["min_range"] = min_range_;
	cfg["max_range"] = max_range_;
	cfg["damage"] = damage_;
	cfg["number"] = num_attacks_;
	cfg["attack_weight"] = attack_weight_;
	cfg["defense_weight"] = defense_weight_;
	cfg["accuracy"] = accuracy_;
	cfg["movement_used"] = movement_used_;
	cfg["parry"] = parry_;
	cfg.add_child("specials", specials_);
}
