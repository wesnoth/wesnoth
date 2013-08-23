/*
   Copyright (C) 2013 by Andrius Silinskas <silinskas.andrius@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#include "flg_manager.hpp"

#include "config.hpp"
#include "formula_string_utils.hpp"
#include "gettext.hpp"
#include "unit_types.hpp"
#include "wml_separators.hpp"

#include <boost/foreach.hpp>

namespace mp  {

#ifdef LOW_MEM
std::string get_RC_suffix(const std::string&, const int)
{
	return "";
}
#else
std::string get_RC_suffix(const std::string& unit_color, const int color)
{
	return "~RC(" + unit_color + ">" + lexical_cast<std::string>(color + 1) +
		")";
}
#endif


flg_manager::flg_manager(const std::vector<const config*>& era_factions,
	const config& side, const bool map_settings, const bool saved_game,
	const bool first_scenario, const int color) :
	era_factions_(era_factions),
	side_(side),
	map_settings_(map_settings),
	saved_game_(saved_game),
	first_scenario_(first_scenario),
	color_(color),
	available_factions_(),
	choosable_factions_(),
	choosable_leaders_(),
	choosable_genders_(),
	current_faction_(NULL),
	current_leader_("null"),
	current_gender_("null")
{
	init_available_factions();
	init_choosable_factions();

	set_current_faction((unsigned) 0);
}

flg_manager::~flg_manager()
{
}

void flg_manager::set_current_faction(const unsigned index)
{
	assert(index < choosable_factions_.size());
	current_faction_ = choosable_factions_[index];

	update_choosable_leaders();
	set_current_leader(0);
}

void flg_manager::set_current_faction(const std::string& id)
{
	unsigned index = 0;
	BOOST_FOREACH(const config* faction, choosable_factions_) {
		if ((*faction)["id"] == id) {
			break;
		}

		index++;
	}

	set_current_faction(index);
}

void flg_manager::set_current_leader(const unsigned index)
{
	assert(index < choosable_leaders_.size());
	current_leader_ = choosable_leaders_[index];

	update_choosable_genders();
	set_current_gender(0);
}

void flg_manager::set_current_gender(const unsigned index)
{
	assert(index < choosable_genders_.size());
	current_gender_ = choosable_genders_[index];
}

void flg_manager::reset_leader_combo(gui::combo& combo_leader)
{
	std::vector<std::string> leaders;
	BOOST_FOREACH(const std::string& leader, choosable_leaders_) {
		const unit_type* unit = unit_types.find(leader);
		if (unit) {
			leaders.push_back(IMAGE_PREFIX + unit->image() +
				get_RC_suffix(unit->flag_rgb(), color_) +
				COLUMN_SEPARATOR + unit->type_name());
		} else if (leader == "random") {
			leaders.push_back(IMAGE_PREFIX + random_enemy_picture +
				COLUMN_SEPARATOR + _("Random"));
		} else if (leader == "null") {
			leaders.push_back(utils::unicode_em_dash);
		} else {
			leaders.push_back("?");
		}
	}

	combo_leader.enable(leaders.size() > 1 && !saved_game_);

	combo_leader.set_items(leaders);
	combo_leader.set_selected(current_leader_index());
}

void flg_manager::reset_gender_combo(gui::combo& combo_gender)
{
	const unit_type* unit = unit_types.find(current_leader_);

	std::vector<std::string> genders;
	BOOST_FOREACH(const std::string& gender, choosable_genders_) {
		if (gender == unit_race::s_female || gender == unit_race::s_male) {
			if (unit) {
				const unit_type& gender_unit =
					unit->get_gender_unit_type(gender);

				std::string gender_name = (gender == unit_race::s_female) ?
					_("Female ♀") : _("Male ♂");
				genders.push_back(IMAGE_PREFIX + gender_unit.image() +
					get_RC_suffix(gender_unit.flag_rgb(), color_) +
					COLUMN_SEPARATOR + gender_name);
			}
		} else if (gender == "random") {
			genders.push_back(IMAGE_PREFIX + random_enemy_picture +
				COLUMN_SEPARATOR + _("Random"));
		} else if (gender == "null") {
			genders.push_back(utils::unicode_em_dash);
		} else {
			genders.push_back("?");
		}
	}

	combo_gender.enable(genders.size() > 1 && !saved_game_);

	combo_gender.set_items(genders);
	combo_gender.set_selected(current_gender_index());
}

void flg_manager::resolve_random() {
	bool solved_random_faction = false;
	if ((*current_faction_)["random_faction"].to_bool()) {
		std::vector<std::string> faction_choices, faction_excepts;

		faction_choices = utils::split((*current_faction_)["choices"]);
		if (faction_choices.size() == 1 && faction_choices.front() == "") {
			faction_choices.clear();
		}

		faction_excepts = utils::split((*current_faction_)["except"]);
		if (faction_excepts.size() == 1 && faction_excepts.front() == "") {
			faction_excepts.clear();
		}

		// Builds the list of factions eligible for choice
		// (non-random factions).
		std::vector<int> nonrandom_sides;
		int num = -1;
		BOOST_FOREACH(const config* i, available_factions_) {
			++num;
			if (!(*i)["random_faction"].to_bool()) {
				const std::string& faction_id = (*i)["id"];

				if (!faction_choices.empty() &&
					std::find(faction_choices.begin(), faction_choices.end(),
						faction_id) == faction_choices.end()) {
					continue;
				}

				if (!faction_excepts.empty() &&
					std::find(faction_excepts.begin(), faction_excepts.end(),
						faction_id) != faction_excepts.end()) {
					continue;
				}

				nonrandom_sides.push_back(num);
			}
		}

		if (nonrandom_sides.empty()) {
			throw config::error(_("Only random sides in the current era."));
		}

		const int faction_index =
			nonrandom_sides[rand() % nonrandom_sides.size()];
		current_faction_ = available_factions_[faction_index];

		update_choosable_leaders();
		set_current_leader(0);

		solved_random_faction = true;
	}

	bool solved_random_leader = false;
	if (current_leader_ == "random" || solved_random_faction) {
		std::vector<std::string> nonrandom_leaders;
		BOOST_FOREACH(const std::string& leader, choosable_leaders_) {
			if (leader != "random") {
				nonrandom_leaders.push_back(leader);
			}
		}

		if (nonrandom_leaders.empty()) {
			utils::string_map i18n_symbols;
			i18n_symbols["faction"] = (*current_faction_)["name"];
			throw config::error(vgettext(
				"Unable to find a leader type for faction $faction",
				i18n_symbols));
		} else {
			const int lchoice = rand() % nonrandom_leaders.size();
			set_current_leader(nonrandom_leaders[lchoice]);
		}

		solved_random_leader = true;
	}

	// Resolve random genders "very much" like standard unit code.
	if (current_gender_ == "random" || solved_random_leader) {
		const unit_type *ut = unit_types.find(current_leader_);
		if (ut && !choosable_genders_.empty()) {
			std::vector<std::string> nonrandom_genders;
			BOOST_FOREACH(const std::string& gender, choosable_genders_) {
				if (gender != "random") {
					nonrandom_genders.push_back(gender);
				}
			}

			const int gchoice = rand() % nonrandom_genders.size();
			set_current_gender(nonrandom_genders[gchoice]);
		} else {
			utils::string_map i18n_symbols;
			i18n_symbols["leader"] = current_leader_;
			throw config::error(vgettext(
				"Cannot obtain genders for invalid leader $leader",
				i18n_symbols));
		}
	}
}

void flg_manager::init_available_factions()
{
	BOOST_FOREACH(const config* faction, era_factions_) {
		const bool has_no_recruits =
			side_["recruit"].empty() && side_["previous_recruits"].empty();
		if ((*faction)["id"] == "Custom" && side_["faction"] != "Custom" &&
			(has_no_recruits || (!map_settings_ && first_scenario_))) {

			// "Custom" faction should not be available if both "recruit"
			// and "previous_recruits" lists are empty or
			// if map settings are not in use. However, it should be
			// available if it was explicitly stated so.
			continue;
		}

		available_factions_.push_back(faction);
	}

	assert(!available_factions_.empty());
}

void flg_manager::init_choosable_factions()
{
	choosable_factions_ = available_factions_;

	if (!side_["faction"].empty() && (map_settings_ || !first_scenario_)) {
		int faction_index = find_suitable_faction();
		if (faction_index >= 0) {
			const config* faction = choosable_factions_[faction_index];
			choosable_factions_.clear();
			choosable_factions_.push_back(faction);
		}
	}
}

void flg_manager::update_choosable_leaders()
{
	choosable_leaders_.clear();

	if (saved_game_) {
		// TODO: find a proper way to determine
		// a leader from a savegame.
		std::string leader;
		BOOST_FOREACH(const config& side_unit, side_.child_range("unit")) {
			if (side_unit["canrecruit"].to_bool()) {
				leader = side_unit["type"].str();
				break;
			}
		}
		if (!leader.empty()) {
			const unit_type *unit = unit_types.find(leader);
			if (unit) {
				choosable_leaders_.push_back(leader);
			}
		}
	} else {
		const std::string& leader_id = side_["id"];
		std::string leader_type = side_["type"];

		if (!leader_id.empty()) {
			// Check if leader was carried over and now is in
			// [unit] tag.
			const config& leader_unit = side_.find_child("unit", "id",
				leader_id);
			if (leader_unit) {
				leader_type = leader_unit["type"].str();
			}
		}

		if ((map_settings_ || !first_scenario_) && !leader_type.empty() &&
			leader_type != "null" && leader_type != "random") {

			// Leader was explicitly assigned.
			const unit_type *unit = unit_types.find(leader_type);
			if (unit) {
				choosable_leaders_.push_back(leader_type);
			}
		} else if ((*current_faction_)["id"] == "Custom") {
			// Allow user to choose a leader from any faction.
			choosable_leaders_.push_back("random");

			BOOST_FOREACH(const config* f, available_factions_) {
				if ((*f)["id"] != "Random") {
					append_leaders_from_faction(f);
				}
			}
		} else if ((*current_faction_)["id"] != "Random") {
			// Faction leader list consists of "random" + "leader=".
			choosable_leaders_.push_back("random");

			append_leaders_from_faction(current_faction_);
		}
	}

	// If none of the possible leaders could be determined,
	// use "null" as an indicator for empty leaders list.
	if (choosable_leaders_.empty()) {
		choosable_leaders_.push_back("null");
	}
}

void flg_manager::update_choosable_genders()
{
	choosable_genders_.clear();

	if (saved_game_) {
		std::string gender;
		BOOST_FOREACH(const config& side_unit, side_.child_range("unit")) {
			if (current_leader_ == side_unit["type"] &&
				side_unit["canrecruit"].to_bool()) {

				gender = side_unit["gender"].str();
				break;
			}
		}
		if (!gender.empty()) {
			choosable_genders_.push_back(gender);
		}
	} else {
		const unit_type* unit = unit_types.find(current_leader_);
		if (unit) {
			const std::string& default_gender = side_["gender"];
			if ((map_settings_ || !first_scenario_) &&
				(default_gender == unit_race::s_female ||
				default_gender == unit_race::s_male)) {

				// Gender was explicitly assigned.
				const unit_type *unit = unit_types.find(current_leader_);
				if (unit) {
					BOOST_FOREACH(unit_race::GENDER gender, unit->genders()) {
						if (default_gender == gender_string(gender)) {
							choosable_genders_.push_back(default_gender);
							break;
						}
					}
				}
			} else {
				if (unit->genders().size() > 1) {
					choosable_genders_.push_back("random");
				}

				BOOST_FOREACH(unit_race::GENDER gender, unit->genders()) {
					if (gender == unit_race::FEMALE) {
						choosable_genders_.push_back(unit_race::s_female);
					} else {
						choosable_genders_.push_back(unit_race::s_male);
					}
				}
			}
		}
	}

	// If none of the possible genders could be determined,
	// use "null" as an indicator for empty genders list.
	if (choosable_genders_.empty()) {
		choosable_genders_.push_back("null");
	}
}

int flg_manager::find_suitable_faction() const
{
	std::vector<std::string> find;
	std::string search_field;
	if (const config::attribute_value *f = side_.get("faction")) {
		// Choose based on faction.
		find.push_back(f->str());
		search_field = "id";
	} else if (side_["faction_from_recruit"].to_bool()) {
		// Choose based on recruit.
		find = utils::split(side_["recruit"]);
		search_field = "recruit";
	} else if (const config::attribute_value *l = side_.get("leader")) {
		// Choose based on leader.
		find.push_back(*l);
		search_field = "leader";
	} else {
		return -1;
	}

	int res = -1, index = 0, best_score = 0;
	BOOST_FOREACH(const config *faction, choosable_factions_)
	{
		int faction_score = 0;
		std::vector<std::string> recruit =
			utils::split((*faction)[search_field]);
		BOOST_FOREACH(const std::string &search, find) {
			BOOST_FOREACH(const std::string &r, recruit) {
				if (r == search) {
					++faction_score;
					break;
				}
			}
		}
		if (faction_score > best_score) {
			best_score = faction_score;
			res = index;
		}
		++index;
	}
	return res;
}

void flg_manager::append_leaders_from_faction(const config* faction)
{
	std::vector<std::string> leaders_to_append =
		utils::split((*faction)["random_leader"]);
	if (leaders_to_append.empty()) {
		leaders_to_append = utils::split((*faction)["leader"]);
	}

	choosable_leaders_.insert(choosable_leaders_.end(), leaders_to_append.begin(),
		leaders_to_append.end());
}

int flg_manager::faction_index(const config* faction) const
{
	std::vector<const config*>::const_iterator it = std::find(
		choosable_factions_.begin(), choosable_factions_.end(), faction);

	assert(it != choosable_factions_.end());
	return std::distance(choosable_factions_.begin(), it);
}

int flg_manager::leader_index(const std::string& leader) const
{
	std::vector<std::string>::const_iterator it = std::find(
		choosable_leaders_.begin(), choosable_leaders_.end(), leader);

	assert(it != choosable_leaders_.end());
	return std::distance(choosable_leaders_.begin(), it);
}

int flg_manager::gender_index(const std::string& gender) const
{
	std::vector<std::string>::const_iterator it = std::find(
		choosable_genders_.begin(), choosable_genders_.end(), gender);

	assert(it != choosable_genders_.end());
	return std::distance(choosable_genders_.begin(), it);
}

} // end namespace mp
