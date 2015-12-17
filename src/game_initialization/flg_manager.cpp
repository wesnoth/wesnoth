/*
   Copyright (C) 2013 - 2015 by Andrius Silinskas <silinskas.andrius@gmail.com>
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
#include "mt_rng.hpp"
#include "unit_types.hpp"
#include "wml_separators.hpp"
#include "log.hpp"

#include <boost/foreach.hpp>

static lg::log_domain log_mp_connect_engine("mp/connect/engine");
#define DBG_MP LOG_STREAM(debug, log_mp_connect_engine)
#define LOG_MP LOG_STREAM(info, log_mp_connect_engine)
#define WRN_MP LOG_STREAM(warn, log_mp_connect_engine)
#define ERR_MP LOG_STREAM(err, log_mp_connect_engine)

namespace ng  {

#ifdef LOW_MEM
std::string get_RC_suffix(const std::string&, const int)
{
	return "";
}
#else
std::string get_RC_suffix(const std::string& unit_color, const std::string& color)
{
	return "~RC(" + unit_color + ">" + color  +
		")";
}
#endif


flg_manager::flg_manager(const std::vector<const config*>& era_factions,
	const config& side, const bool lock_settings, const bool use_map_settings,
	const bool saved_game) :
	era_factions_(era_factions),
	side_(side),
	use_map_settings_(use_map_settings),
	saved_game_(saved_game),
	has_no_recruits_(get_original_recruits(side_).empty() && side_["previous_recruits"].empty()),
	faction_lock_(side_["faction_lock"].to_bool(lock_settings) && use_map_settings),
	leader_lock_(side_["leader_lock"].to_bool(lock_settings) && use_map_settings),
	available_factions_(),
	available_leaders_(),
	available_genders_(),
	choosable_factions_(),
	choosable_leaders_(),
	choosable_genders_(),
	current_faction_(NULL),
	current_leader_("null"),
	current_gender_("null"),
	default_leader_type_(side_["type"]),
	default_leader_gender_(side_["gender"]),
	default_leader_cfg_(NULL)
{
	const std::string& leader_id = side_["id"];
	if (!leader_id.empty()) {
		// Check if leader was carried over and now is in [unit] tag.
		default_leader_cfg_ = &side_.find_child("unit", "id", leader_id);
		if (*default_leader_cfg_) {
			default_leader_type_ = (*default_leader_cfg_)["type"].str();
			default_leader_gender_ = (*default_leader_cfg_)["gender"].str();
		} else {
			default_leader_cfg_ = NULL;
		}
	} else if (default_leader_type_.empty()) {
		// Find a unit which can recruit.
		BOOST_FOREACH(const config& side_unit, side_.child_range("unit")) {
			if (side_unit["canrecruit"].to_bool()) {
				default_leader_type_ = side_unit["type"].str();
				default_leader_gender_ = side_unit["gender"].str();
				default_leader_cfg_ = &side_unit;
				break;
			}
		}
	}
	if (!default_leader_type_.empty() && default_leader_type_ != "random") {
		const unit_type* unit = unit_types.find(default_leader_type_);
		if (unit == NULL) {
			default_leader_type_.clear();
			default_leader_gender_.clear();
			default_leader_cfg_ = NULL;
		}
	}

	update_available_factions();

	set_current_faction(0);
}

flg_manager::~flg_manager()
{
}

void flg_manager::set_current_faction(const unsigned index)
{
	assert(index < choosable_factions_.size());
	current_faction_ = choosable_factions_[index];

	update_available_leaders();
	set_current_leader(0);
}

void flg_manager::set_current_faction(const std::string& id)
{
	unsigned index = 0;
	BOOST_FOREACH(const config* faction, choosable_factions_) {
		if ((*faction)["id"] == id) {
			set_current_faction(index);
			return;
		}
		index++;
	}
	ERR_MP << "Faction '" << id << "' is not available for side " << side_["side"] << " Ignoring";

}

void flg_manager::set_current_leader(const unsigned index)
{
	assert(index < choosable_leaders_.size());
	current_leader_ = choosable_leaders_[index];

	update_available_genders();
	set_current_gender(0);
}

void flg_manager::set_current_gender(const unsigned index)
{
	assert(index < choosable_genders_.size());
	current_gender_ = choosable_genders_[index];
}

void flg_manager::reset_leader_combo(gui::combo& combo_leader, const std::string& color) const
{
	std::vector<std::string> leaders;
	BOOST_FOREACH(const std::string& leader, choosable_leaders_) {
		const unit_type* unit = unit_types.find(leader);
		if (unit) {
			leaders.push_back(IMAGE_PREFIX + unit->image() +
				get_RC_suffix(unit->flag_rgb(), color) +
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

void flg_manager::reset_gender_combo(gui::combo& combo_gender, const std::string& color) const
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
					get_RC_suffix(gender_unit.flag_rgb(), color) +
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

bool flg_manager::is_random_faction()
{
	return (*current_faction_)["random_faction"].to_bool();
}

// When we use a random mode like "no mirror", "no ally mirror", the list of
// faction ids to avoid is past as an argument.
// It may be that for some scenario configuration, a strict no mirror
// assignment is not possible, because there are too many sides, or some users
// have forced their faction choices to be matching, etc.
// In that case we gracefully continue by ignoring the no mirror rule and
// assigning as we would have if it were off.
// If there is still no options we throw a config error because it means the
// era is misconfigured.
void flg_manager::resolve_random(rand_rng::mt_rng & rng, const std::vector<std::string> & avoid) {
	if (is_random_faction()) {
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
		std::vector<int> fallback_nonrandom_sides;
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

				fallback_nonrandom_sides.push_back(num); //This side is consistent with this random faction, remember as a fallback.

				if (!avoid.empty() &&
					std::find(avoid.begin(), avoid.end(),
						faction_id) != avoid.end()) {
					continue;
				}

				nonrandom_sides.push_back(num); //This side is consistent with this random faction, and the avoid factions argument.
			}
		}

		if (nonrandom_sides.empty()) {
			nonrandom_sides = fallback_nonrandom_sides; // There was no way to succeed consistently with the avoid factions argument, so ignore it as a fallback.
		}

		if (nonrandom_sides.empty()) {
			throw config::error(_("Only random sides in the current era."));
		}

		const int faction_index =
			nonrandom_sides[rng.get_next_random() % nonrandom_sides.size()];
		current_faction_ = available_factions_[faction_index];

		update_available_leaders();
		set_current_leader(0);
	}

	if (current_leader_ == "random") {
		std::vector<std::string> nonrandom_leaders =
			utils::split((*current_faction_)["random_leader"]);
		if (nonrandom_leaders.empty()) {
			BOOST_FOREACH(const std::string& leader, available_leaders_) {
				if (leader != "random") {
					nonrandom_leaders.push_back(leader);
				}
			}
		}

		if (nonrandom_leaders.empty()) {
			utils::string_map i18n_symbols;
			i18n_symbols["faction"] = (*current_faction_)["name"];
			throw config::error(vgettext(
				"Unable to find a leader type for faction $faction",
				i18n_symbols));
		} else {
			const int lchoice = rng.get_next_random() % nonrandom_leaders.size();
			current_leader_ = nonrandom_leaders[lchoice];

			update_available_genders();
			set_current_gender(0);
		}
	}

	// Resolve random genders "very much" like standard unit code.
	if (current_gender_ == "random") {
		const unit_type *ut = unit_types.find(current_leader_);
		if (ut) {
			std::vector<std::string> nonrandom_genders;
			BOOST_FOREACH(const std::string& gender, available_genders_) {
				if (gender != "random") {
					nonrandom_genders.push_back(gender);
				}
			}

			const int gchoice = rng.get_next_random() % nonrandom_genders.size();
			current_gender_ = nonrandom_genders[gchoice];
		} else {
			utils::string_map i18n_symbols;
			i18n_symbols["leader"] = current_leader_;
			throw config::error(vgettext(
				"Cannot obtain genders for invalid leader $leader",
				i18n_symbols));
		}
	}
}

void flg_manager::update_available_factions()
{
	const config* custom_faction = NULL;
	const bool show_custom_faction = side_["faction"] == "Custom" || !has_no_recruits_ || faction_lock_;

	BOOST_FOREACH(const config* faction, era_factions_) {
		if ((*faction)["id"] == "Custom" && !show_custom_faction) {

			// "Custom" faction should not be available if both
			// "default_recruit" and "previous_recruits" lists are empty.
			// However, it should be available if it was explicitly stated so.
			custom_faction = faction;
			continue;
		}

		// Add default faction to the top of the list.
		if (side_["faction"] == (*faction)["id"]) {
			available_factions_.insert(available_factions_.begin(), faction);
		} else {
			available_factions_.push_back(faction);
		}
	}
	if (available_factions_.empty() && custom_faction) {
		available_factions_.push_back(custom_faction);
	}

	assert(!available_factions_.empty());

	update_choosable_factions();
}

void flg_manager::update_available_leaders()
{
	available_leaders_.clear();

	if (!side_["no_leader"].to_bool() || !leader_lock_) {

		int random_pos = 0;
		// Add a default leader if there is one.
		if (!default_leader_type_.empty()) {
			available_leaders_.push_back(default_leader_type_);
			random_pos = 1;
		}

		if (!saved_game_) {
			if (!is_random_faction()) {
				if ((*current_faction_)["id"] == "Custom") {
					// Allow user to choose a leader from any faction.
					BOOST_FOREACH(const config* f, available_factions_) {
						if ((*f)["id"] != "Random") {
							append_leaders_from_faction(f);
						}
					}
				} else {
					append_leaders_from_faction(current_faction_);
				}

				// Remove duplicate leaders.
				std::set<std::string> seen;
				std::vector<std::string>::iterator walker, modifier;
				for(walker = available_leaders_.begin(),
					modifier = available_leaders_.begin();
					walker != available_leaders_.end(); ++walker) {

					if (seen.insert(*walker).second) {
						*modifier++ = *walker;
					}
				}

				available_leaders_.erase(modifier, available_leaders_.end());

				if (!available_leaders_.empty())
					available_leaders_.insert(available_leaders_.begin() + random_pos, "random");
			}
		}
	}

	// If none of the possible leaders could be determined,
	// use "null" as an indicator for empty leaders list.
	if (available_leaders_.empty()) {
		available_leaders_.push_back("null");
	}

	update_choosable_leaders();
}

void flg_manager::update_available_genders()
{
	available_genders_.clear();

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
			available_genders_.push_back(gender);
		}
	} else {
		const unit_type* unit = unit_types.find(current_leader_);
		if (unit) {
			if (unit->genders().size() > 1 && !leader_lock_) {
				available_genders_.push_back("random");
			}

			BOOST_FOREACH(unit_race::GENDER gender, unit->genders()) {
				std::string gender_str;
				if (gender == unit_race::FEMALE) {
					gender_str = unit_race::s_female;
				} else {
					gender_str = unit_race::s_male;
				}

				// Add default gender to the top of the list.
				if (default_leader_gender_ == gender_str) {
					available_genders_.insert(available_genders_.begin(),
						gender_str);
				} else {
					available_genders_.push_back(gender_str);
				}
			}
		}
	}

	// If none of the possible genders could be determined,
	// use "null" as an indicator for empty genders list.
	if (available_genders_.empty()) {
		available_genders_.push_back("null");
	}

	update_choosable_genders();
}

void flg_manager::update_choosable_factions()
{
	choosable_factions_ = available_factions_;

	if (faction_lock_) {;
		const int faction_index = find_suitable_faction();
		if (faction_index >= 0) {
			const config* faction = choosable_factions_[faction_index];
			choosable_factions_.clear();
			choosable_factions_.push_back(faction);
		}
	}
}

void flg_manager::update_choosable_leaders()
{
	choosable_leaders_ = available_leaders_;

	if (!default_leader_type_.empty() && leader_lock_) {
		if (std::find(available_leaders_.begin(), available_leaders_.end(),
			default_leader_type_) != available_leaders_.end()) {

			choosable_leaders_.clear();
			choosable_leaders_.push_back(default_leader_type_);
		}
	}
}

void flg_manager::update_choosable_genders()
{
	choosable_genders_ = available_genders_;

	if (leader_lock_) {
		std::string default_gender = default_leader_gender_;
		if (default_gender.empty()) {
			default_gender = choosable_genders_.front();
		}

		if (std::find(available_genders_.begin(), available_genders_.end(),
			default_gender) != available_genders_.end()) {

			choosable_genders_.clear();
			choosable_genders_.push_back(default_gender);
		}
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
		find = get_original_recruits(side_);
		search_field = "recruit";
	} else if (const config::attribute_value *l = side_.get("leader")) {
		// Choose based on leader.
		find.push_back(*l);
		search_field = "leader";
	} else {
		find.push_back("Custom");
		search_field = "id";
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

int flg_manager::current_faction_index() const
{
	assert(current_faction_);

	return faction_index(*current_faction_);
}

void flg_manager::append_leaders_from_faction(const config* faction)
{
	std::vector<std::string> leaders_to_append =
		utils::split((*faction)["leader"]);

	available_leaders_.insert(available_leaders_.end(), leaders_to_append.begin(),
		leaders_to_append.end());
}

int flg_manager::faction_index(const config& faction) const
{
	std::vector<const config*>::const_iterator it = std::find(
		choosable_factions_.begin(), choosable_factions_.end(), &faction);

	assert(it != choosable_factions_.end());
	return std::distance(choosable_factions_.begin(), it);
}

int flg_manager::leader_index(const std::string& leader) const
{
	std::vector<std::string>::const_iterator it = std::find(
		choosable_leaders_.begin(), choosable_leaders_.end(), leader);

	return it != choosable_leaders_.end() ? std::distance(choosable_leaders_.begin(), it) : -1;
}

int flg_manager::gender_index(const std::string& gender) const
{
	std::vector<std::string>::const_iterator it = std::find(
		choosable_genders_.begin(), choosable_genders_.end(), gender);

	return it != choosable_genders_.end() ? std::distance(choosable_genders_.begin(), it) : -1;
}

void flg_manager::set_current_leader(const std::string& leader)
{
	int index = leader_index(leader);
	if (index < 0) {
		ERR_MP << "Leader '" << leader << "' is not available for side " << side_["side"] << " Ignoring";
	}
	else {
		set_current_leader(index);
	}
}

void flg_manager::set_current_gender(const std::string& gender)
{
	int index = gender_index(gender);
	if (index < 0) {
		ERR_MP << "Gender '" << gender << "' is not available for side " << side_["side"] << " Ignoring";
	}
	else {
		set_current_gender(index);
	}
}

std::vector<std::string> flg_manager::get_original_recruits(const config& cfg)
{
	if (cfg["default_recruit"].empty()) {
		return std::vector<std::string>();
	}
	const config::attribute_value& cfg_default_recruit = cfg["default_recruit"];
	if (!cfg_default_recruit.empty()) {
		return utils::split(cfg_default_recruit.str());
	}
	else {
		return utils::split(cfg["recruit"].str());
	}
}
} // end namespace ng
