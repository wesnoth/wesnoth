/* $Id$ */
/*
   Copyright (C) 2007
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "gettext.hpp"
#include "leader_list.hpp"
#include "wml_separators.hpp"
#include "serialization/string_utils.hpp"
#include "widgets/combo.hpp"
#include "widgets/menu.hpp"

const std::string leader_list_manager::random_enemy_picture("random-enemy.png");

leader_list_manager::leader_list_manager(const config::child_list& side_list,
		const game_data* data, gui::combo* leader_combo , gui::combo* gender_combo):
	side_list_(side_list), data_(data), leader_combo_(leader_combo), gender_combo_(gender_combo)
{
}

void leader_list_manager::set_leader_combo(gui::combo* combo)
{
	int selected = leader_combo_ != NULL ? leader_combo_->selected() : 0;
	leader_combo_ = combo;

	if(leader_combo_ != NULL) {
		if(leaders_.empty()) {
			update_leader_list(0);
		} else {
			populate_leader_combo(selected);
		}
	}
}

void leader_list_manager::set_gender_combo(gui::combo* combo)
{
	gender_combo_ = combo;

	if(gender_combo_ != NULL) {
		if(!leaders_.empty()) {
			update_gender_list(get_leader());
		}
	}
}

void leader_list_manager::update_leader_list(int side_index)
{
	const config& side = *side_list_[side_index];

	leaders_.clear();

	if(side["random_faction"] == "yes") {
		if(leader_combo_ != NULL) {
			std::vector<std::string> dummy;
			dummy.push_back("-");
			leader_combo_->enable(false);
			leader_combo_->set_items(dummy);
			leader_combo_->set_selected(0);
		}
		return;
	} else {
		if(leader_combo_ != NULL)
			leader_combo_->enable(true);
		if(gender_combo_ != NULL)
			gender_combo_->enable(true);
	}

	if(!side["leader"].empty()) {
		leaders_ = utils::split(side["leader"]);
	}

	const std::string default_leader = side["type"];
	size_t default_index = 0;

	std::vector<std::string>::const_iterator itor;

	for (itor = leaders_.begin(); itor != leaders_.end(); ++itor) {
		if (*itor == default_leader) {
			break;
		}
		default_index++;
	}

	if (default_index == leaders_.size()) {
		leaders_.push_back(default_leader);
	}

	leaders_.push_back("random");
	populate_leader_combo(default_index);
}

void leader_list_manager::update_gender_list(const std::string& leader)
{
	int gender_index = gender_combo_ != NULL ? gender_combo_->selected() : 0;
	genders_.clear();
	gender_ids_.clear();
	if (leader == "random" || leader == "-" || leader == "?") {
		// Assume random/unknown leader/faction == unknown gender
		gender_ids_.push_back("null");
		genders_.push_back("-");
		gender_combo_->enable(false);
		gender_combo_->set_items(genders_);
		gender_combo_->set_selected(0);
		return;
	}

	const game_data::unit_type_map& utypes = data_->unit_types;
	if (utypes.find(leader) != utypes.end()) {
		const unit_type* ut;
		const unit_type* utg;
		ut = &(utypes.find(leader)->second);
		const std::vector<unit_race::GENDER> genders = ut->genders();
		if (genders.size() < 2) {
			gender_combo_->enable(false);
		} else {
			gender_ids_.push_back("random");
			genders_.push_back(IMAGE_PREFIX + random_enemy_picture + COLUMN_SEPARATOR + _("gender^Random"));
			gender_combo_->enable(true);
		}
		for (std::vector<unit_race::GENDER>::const_iterator i=genders.begin(); i != genders.end(); ++i) {
			utg = &(ut->get_gender_unit_type(*i));

			// Make the internationalized titles for each gender, along with the WML ids
			if (*i == unit_race::FEMALE)
			{
				gender_ids_.push_back("female");
#ifdef LOW_MEM
				genders_.push_back(IMAGE_PREFIX + utg->image() + COLUMN_SEPARATOR + _("Female ♀"));
#else
				genders_.push_back(IMAGE_PREFIX + utg->image() + std::string("~RC(" + utg->flag_rgb() + ">1)") + COLUMN_SEPARATOR + _("Female ♀"));
#endif
			}
			else
			{
				gender_ids_.push_back("male");
#ifdef LOW_MEM
				genders_.push_back(IMAGE_PREFIX + utg->image() + COLUMN_SEPARATOR + _("Male ♂"));
#else
				genders_.push_back(IMAGE_PREFIX + utg->image() + std::string("~RC(" + utg->flag_rgb() + ">1)") + COLUMN_SEPARATOR + _("Male ♂"));
#endif
			}
		}
		gender_combo_->set_items(genders_);
		gender_index %= genders_.size();
		gender_combo_->set_selected(gender_index);	
	} else {
		gender_ids_.push_back("random");
		genders_.push_back(IMAGE_PREFIX + random_enemy_picture + COLUMN_SEPARATOR + _("Random"));
		gender_combo_->enable(false);
		gender_combo_->set_items(genders_);
		gender_combo_->set_selected(0);	
	}
}

void leader_list_manager::populate_leader_combo(int selected_index) {
	std::vector<std::string>::const_iterator itor;
	std::vector<std::string> leader_strings;
	for(itor = leaders_.begin(); itor != leaders_.end(); ++itor) {

		const game_data::unit_type_map& utypes = data_->unit_types;

		//const std::string name = data_->unit_types->find(*itor).language_name();
		if (utypes.find(*itor) != utypes.end()) {
			const unit_type* ut;
			ut = &(utypes.find(*itor)->second);
			if (gender_combo_ != NULL && !genders_.empty() && size_t(gender_combo_->selected()) < genders_.size()) {
				if (gender_ids_[gender_combo_->selected()] == "male"){
					ut = &(utypes.find(*itor)->second.get_gender_unit_type(unit_race::MALE));
				} else if (gender_ids_[gender_combo_->selected()] == "female") {
					ut = &(utypes.find(*itor)->second.get_gender_unit_type(unit_race::FEMALE));
				}
			}
			const std::string name =  ut->language_name();
			const std::string image = ut->image();

#ifdef LOW_MEM
			leader_strings.push_back(IMAGE_PREFIX + image + COLUMN_SEPARATOR + name);
#else
			leader_strings.push_back(IMAGE_PREFIX + image + std::string("~RC(" + ut->flag_rgb() + ">1)") + COLUMN_SEPARATOR + name);
#endif
		} else {
			if(*itor == "random") {
				leader_strings.push_back(IMAGE_PREFIX + random_enemy_picture + COLUMN_SEPARATOR + _("Random"));
			} else {
				leader_strings.push_back("?");
			}
		}
	}

	if(leader_combo_ != NULL) {
		leader_combo_->set_items(leader_strings);
		leader_combo_->set_selected(selected_index);
	}
}

void leader_list_manager::set_leader(const std::string& leader)
{
	if(leader_combo_ == NULL)
		return;

	int leader_index = 0;
	for(std::vector<std::string>::const_iterator itor = leaders_.begin();
			itor != leaders_.end(); ++itor) {
		if(leader == *itor) {
			leader_combo_->set_selected(leader_index);
			return;
		}
		++leader_index;
	}
}

void leader_list_manager::set_gender(const std::string& gender)
{
	if(gender_combo_ == NULL)
		return;

	int gender_index = 0;
	for(std::vector<std::string>::const_iterator itor = gender_ids_.begin();
			itor != gender_ids_.end(); ++itor) {
		if(gender == *itor) {
			gender_combo_->set_selected(gender_index);
			return;
		}
		++gender_index;
	}
}

std::string leader_list_manager::get_leader() const
{
	if(leader_combo_ == NULL)
		return _("?");

	if(leaders_.empty())
		return "random";

	if(size_t(leader_combo_->selected()) >= leaders_.size())
		return _("?");

	return leaders_[leader_combo_->selected()];
}

std::string leader_list_manager::get_gender() const
{
	if(gender_combo_ == NULL || genders_.empty() || size_t(gender_combo_->selected()) >= genders_.size())
		return "null";
	return gender_ids_[gender_combo_->selected()];
}
