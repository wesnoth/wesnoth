/*
	Copyright (C) 2013 - 2024
	by Andrius Silinskas <silinskas.andrius@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "game_initialization/flg_manager.hpp"

#include "config.hpp"
#include "formula/string_utils.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "mt_rng.hpp"
#include "units/types.hpp"
#include "utils/general.hpp"

#include <algorithm>

static lg::log_domain log_mp_connect_engine("mp/connect/engine");
#define LOG_MP LOG_STREAM(info, log_mp_connect_engine)
#define ERR_MP LOG_STREAM(err, log_mp_connect_engine)

namespace ng
{

flg_manager::flg_manager(const std::vector<const config*>& era_factions,
		const config& side, const bool lock_settings, const bool use_map_settings, const bool saved_game)
	: era_factions_(era_factions)
	, side_num_(side["side"].to_int())
	, faction_from_recruit_(side["faction_from_recruit"].to_bool())
	, original_faction_(get_default_faction(side)["faction"].str())
	, original_recruit_(utils::split(get_default_faction(side)["recruit"].str()))
	, saved_game_(saved_game)
	, has_no_recruits_(original_recruit_.empty() && side["previous_recruits"].empty())
	, faction_lock_(side["faction_lock"].to_bool(lock_settings))
	, leader_lock_(side["leader_lock"].to_bool(lock_settings))
	, available_factions_()
	, available_leaders_()
	, available_genders_()
	, choosable_factions_()
	, choosable_leaders_()
	, choosable_genders_()
	, current_faction_(nullptr)
	, current_leader_("null")
	, current_gender_("null")
	, default_leader_type_("")
	, default_leader_gender_("")
{
	std::string leader_id = side["id"];
	bool found_leader;

	leader_lock_ = leader_lock_ && (use_map_settings || lock_settings || default_leader_type_.empty());
	faction_lock_ = faction_lock_ && (use_map_settings || lock_settings);

	auto set_leader = [&](const config& cfg) {
		found_leader = true;
		leader_id = cfg["id"];
		default_leader_type_ = cfg["type"];
		default_leader_gender_ = cfg["gender"];
	};

	if(auto p_cfg = side.optional_child("leader")) {
		set_leader(*p_cfg);
		// If we are not the host of the game it can happen that the code overwrote
		// the [leaders] type/gender and the original values are found in [default_faction]
		// we still need the id from p_cfg tho.
		if(auto p_ocfg = get_default_faction(side).optional_child("leader")) {
			default_leader_type_ = (*p_ocfg)["type"];
			default_leader_gender_ = (*p_ocfg)["gender"];
		}
	}

	if(!leader_id.empty()) {
		// Check if leader was carried over and now is in [unit] tag.
		// (in this case we dont allow changing it, but we still want to show the corect unit type in the dialogs)
		if(auto p_cfg = side.find_child("unit", "id", leader_id)) {
			set_leader(*p_cfg);
			leader_lock_ = true;
		}
	}

	if(!found_leader) {
		// Find a unit which can recruit.
		if(auto p_cfg = side.find_child("unit", "canrecruit", "yes")) {
			set_leader(*p_cfg);
			leader_lock_ = true;
		}
	}


	if(!default_leader_type_.empty() && default_leader_type_ != "random") {
		if(unit_types.find(default_leader_type_) == nullptr) {
			default_leader_type_.clear();
			default_leader_gender_.clear();
		}
	}

	update_available_factions();

	select_default_faction();

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
	for(const config* faction : choosable_factions_) {
		if((*faction)["id"] == id) {
			set_current_faction(index);
			return;
		}
		index++;
	}

	ERR_MP << "Faction '" << id << "' is not available for side " << side_num_ << " Ignoring";
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

bool flg_manager::is_random_faction()
{
	return (*current_faction_)["random_faction"].to_bool();
}

// When we use a random mode like "no mirror", "no ally mirror", the list of faction ids to avoid is passed
// as an argument. It may be that for some scenario configuration, a strict no mirror assignment is not possible,
// because there are too many sides, or some users have forced their faction choices to be matching, etc.
// In that case we gracefully continue by ignoring the no mirror rule and  assigning as we would have if it were off.
// If there is still no options we throw a config error because it means the era is misconfigured.
void flg_manager::resolve_random(randomness::mt_rng& rng, const std::vector<std::string>& avoid)
{
	if(is_random_faction()) {
		std::vector<std::string> faction_choices, faction_excepts;

		faction_choices = utils::split((*current_faction_)["choices"]);
		if(faction_choices.size() == 1 && faction_choices.front().empty()) {
			faction_choices.clear();
		}

		faction_excepts = utils::split((*current_faction_)["except"]);
		if(faction_excepts.size() == 1 && faction_excepts.front().empty()) {
			faction_excepts.clear();
		}

		// Builds the list of factions eligible for choice (non-random factions).
		std::vector<int> nonrandom_sides;
		std::vector<int> fallback_nonrandom_sides;
		for(unsigned int i = 0; i < available_factions_.size(); ++i) {
			const config& faction = *available_factions_[i];

			if(faction["random_faction"].to_bool()) {
				continue;
			}

			const std::string& faction_id = faction["id"];

			if(!faction_choices.empty() && std::find(faction_choices.begin(), faction_choices.end(),
					faction_id) == faction_choices.end()) {
				continue;
			}

			if(!faction_excepts.empty() && std::find(faction_excepts.begin(), faction_excepts.end(),
					faction_id) != faction_excepts.end()) {
				continue;
			}

			// This side is consistent with this random faction, remember as a fallback.
			fallback_nonrandom_sides.push_back(i);

			if(!avoid.empty() && std::find(avoid.begin(), avoid.end(),
					faction_id) != avoid.end()) {
				continue;
			}

			// This side is consistent with this random faction, and the avoid factions argument.
			nonrandom_sides.push_back(i);
		}

		if(nonrandom_sides.empty()) {
			// There was no way to succeed consistently with the avoid factions argument, so ignore it as a fallback.
			nonrandom_sides = fallback_nonrandom_sides;
		}

		if(nonrandom_sides.empty()) {
			throw config::error(_("Only random sides in the current era."));
		}

		const int faction_index = nonrandom_sides[rng.get_next_random() % nonrandom_sides.size()];
		current_faction_ = available_factions_[faction_index];

		update_available_leaders();
		set_current_leader(0);
	}

	if(current_leader_ == "random") {
		std::vector<std::string> nonrandom_leaders = utils::split((*current_faction_)["random_leader"]);
		if(nonrandom_leaders.empty()) {
			for(const std::string& leader : available_leaders_) {
				if(leader != "random") {
					nonrandom_leaders.push_back(leader);
				}
			}
		}

		if(nonrandom_leaders.empty()) {
			throw config::error(VGETTEXT(
				"Unable to find a leader type for faction $faction", {{"faction", (*current_faction_)["name"].str()}}));
		} else {
			const int lchoice = rng.get_next_random() % nonrandom_leaders.size();
			current_leader_ = nonrandom_leaders[lchoice];

			update_available_genders();
			set_current_gender(0);
		}
	}

	// Resolve random genders "very much" like standard unit code.
	if(current_gender_ == "random") {
		if(unit_types.find(current_leader_)) {
			std::vector<std::string> nonrandom_genders;
			for(const std::string& gender : available_genders_) {
				if(gender != "random") {
					nonrandom_genders.push_back(gender);
				}
			}

			const int gchoice = rng.get_next_random() % nonrandom_genders.size();
			current_gender_ = nonrandom_genders[gchoice];
		} else {
			throw config::error(VGETTEXT("Cannot obtain genders for invalid leader $leader", {{"leader", current_leader_}}));
		}
	}
}

void flg_manager::update_available_factions()
{
	const config* custom_faction = nullptr;
	const bool show_custom_faction = original_faction_ == "Custom" || !has_no_recruits_ || faction_lock_;

	for(const config* faction : era_factions_) {
		if((*faction)["id"] == "Custom" && !show_custom_faction) {

			// "Custom" faction should not be available if both
			// "recruit" and "previous_recruits" lists are empty.
			// However, it should be available if it was explicitly stated so.
			custom_faction = faction;
			continue;
		}

		// Add default faction to the top of the list.
		if(original_faction_ == (*faction)["id"]) {
			available_factions_.insert(available_factions_.begin(), faction);
		} else {
			available_factions_.push_back(faction);
		}
	}

	if(available_factions_.empty() && custom_faction) {
		available_factions_.push_back(custom_faction);
	}

	assert(!available_factions_.empty());

	update_choosable_factions();
}

void flg_manager::update_available_leaders()
{
	available_leaders_.clear();

	if(!default_leader_type_.empty() || !leader_lock_) {

		int random_pos = 0;
		// Add a default leader if there is one.
		if(!default_leader_type_.empty()) {
			available_leaders_.push_back(default_leader_type_);
			random_pos = 1;
		}

		if(!saved_game_ && !is_random_faction()) {
			if((*current_faction_)["id"] == "Custom") {
				// Allow user to choose a leader from any faction.
				for(const config* f : available_factions_) {
					if((*f)["id"] != "Random") {
						append_leaders_from_faction(f);
					}
				}
			} else {
				append_leaders_from_faction(current_faction_);
			}

			// Remove duplicate leaders.
			std::set<std::string> seen;
			utils::erase_if(available_leaders_, [&seen](const std::string& s) { return !seen.insert(s).second; });

			if(available_leaders_.size() > 1) {
				available_leaders_.insert(available_leaders_.begin() + random_pos, "random");
			}
		}
	}

	// If none of the possible leaders could be determined,
	// use "null" as an indicator for empty leaders list.
	if(available_leaders_.empty()) {
		available_leaders_.push_back("null");
	}

	update_choosable_leaders();
}

void flg_manager::update_available_genders()
{
	available_genders_.clear();

	if(saved_game_) {
		if(!default_leader_gender_.empty()) {
			available_genders_.push_back(default_leader_gender_);
		}
	} else {
		if(const unit_type* unit = unit_types.find(current_leader_)) {
			if(unit->genders().size() > 1 && !leader_lock_) {
				available_genders_.push_back("random");
			}

			for(unit_race::GENDER gender : unit->genders()) {
				const std::string gender_str = gender == unit_race::FEMALE
					? unit_race::s_female
					: unit_race::s_male;

				// Add default gender to the top of the list.
				if(default_leader_gender_ == gender_str) {
					available_genders_.insert(available_genders_.begin(), gender_str);
				} else {
					available_genders_.push_back(gender_str);
				}
			}
		}
	}

	// If none of the possible genders could be determined,
	// use "null" as an indicator for empty genders list.
	if(available_genders_.empty()) {
		available_genders_.push_back("null");
	}

	update_choosable_genders();
}

void flg_manager::update_choosable_factions()
{
	choosable_factions_ = available_factions_;

	if(faction_lock_) {;
		const int faction_index = find_suitable_faction();
		if(faction_index >= 0) {
			const config* faction = choosable_factions_[faction_index];
			choosable_factions_.clear();
			choosable_factions_.push_back(faction);
		}
	}
}

void flg_manager::update_choosable_leaders()
{
	choosable_leaders_ = available_leaders_;

	if(!default_leader_type_.empty() && leader_lock_) {
		if(std::find(available_leaders_.begin(), available_leaders_.end(),
			default_leader_type_) != available_leaders_.end()) {

			choosable_leaders_.clear();
			choosable_leaders_.push_back(default_leader_type_);
		}
	}

	// Sort alphabetically, but with the 'random' option always first
	std::sort(choosable_leaders_.begin() + 1, choosable_leaders_.end(), [](const std::string& str1, const std::string& str2) {
		return str1 < str2;
	});
}

void flg_manager::update_choosable_genders()
{
	choosable_genders_ = available_genders_;

	if(leader_lock_) {
		std::string default_gender = default_leader_gender_;
		if(default_gender.empty()) {
			default_gender = choosable_genders_.front();
		}

		if(std::find(available_genders_.begin(), available_genders_.end(), default_gender) != available_genders_.end()) {
			choosable_genders_.clear();
			choosable_genders_.push_back(default_gender);
		}
	}
}

void flg_manager::select_default_faction()
{
	const std::string& default_faction = original_faction_;
	auto default_faction_it = std::find_if(choosable_factions_.begin(), choosable_factions_.end(),
		[&default_faction](const config* faction) {
			return (*faction)["id"] == default_faction;
		});

	if(default_faction_it != choosable_factions_.end()) {
		set_current_faction(std::distance(choosable_factions_.begin(), default_faction_it));
	} else {
		set_current_faction(0u);
	}
}

int flg_manager::find_suitable_faction() const
{
	std::vector<std::string> find;
	std::string search_field;

	if(!original_faction_.empty()) {
		// Choose based on faction.
		find.push_back(original_faction_);
		search_field = "id";
	} else if(faction_from_recruit_) {
		// Choose based on recruit.
		find = original_recruit_;
		search_field = "recruit";
	} else {
		find.push_back("Custom");
		search_field = "id";
	}

	int res = -1, index = 0, best_score = 0;
	for(const config* faction : choosable_factions_) {
		int faction_score = 0;
		for(const std::string& search : find) {
			for(const std::string& r : utils::split((*faction)[search_field])) {
				if(r == search) {
					++faction_score;
					break;
				}
			}
		}

		if(faction_score > best_score) {
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
	std::vector<std::string> leaders_to_append = utils::split((*faction)["leader"]);

	available_leaders_.insert(available_leaders_.end(), leaders_to_append.begin(),
		leaders_to_append.end());
}

int flg_manager::faction_index(const config& faction) const
{
	const auto it = std::find(choosable_factions_.begin(), choosable_factions_.end(), &faction);

	assert(it != choosable_factions_.end());
	return std::distance(choosable_factions_.begin(), it);
}

int flg_manager::leader_index(const std::string& leader) const
{
	const auto it = std::find(choosable_leaders_.begin(), choosable_leaders_.end(), leader);

	return it != choosable_leaders_.end() ? std::distance(choosable_leaders_.begin(), it) : -1;
}

int flg_manager::gender_index(const std::string& gender) const
{
	const auto it = std::find(choosable_genders_.begin(), choosable_genders_.end(), gender);

	return it != choosable_genders_.end() ? std::distance(choosable_genders_.begin(), it) : -1;
}

void flg_manager::set_current_leader(const std::string& leader)
{
	int index = leader_index(leader);
	if(index < 0) {
		ERR_MP << "Leader '" << leader << "' is not available for side " << side_num_ << " Ignoring";
	} else {
		set_current_leader(index);
	}
}

void flg_manager::set_current_gender(const std::string& gender)
{
	int index = gender_index(gender);
	if(index < 0) {
		ERR_MP << "Gender '" << gender << "' is not available for side " << side_num_ << " Ignoring";
	} else {
		set_current_gender(index);
	}
}

const config& flg_manager::get_default_faction(const config& cfg)
{
	if(auto df = cfg.optional_child("default_faction")) {
		return *df;
	} else {
		return cfg;
	}
}

} // end namespace ng
