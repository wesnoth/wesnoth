/* $Id$ */
/*
   Copyright (C) 2007 - 2010
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * Manage the selection of a leader, and select his/her gender.
 */

#include "global.hpp"

#include "gettext.hpp"
#include "leader_list.hpp"
#include "wml_separators.hpp"
#include "widgets/combo.hpp"

const std::string leader_list_manager::random_enemy_picture("units/random-dice.png");

leader_list_manager::leader_list_manager(const std::vector<const config *> &side_list,
		gui::combo* leader_combo , gui::combo* gender_combo):
	leaders_(),
	genders_(),
	gender_ids_(),
	side_list_(side_list),
	leader_combo_(leader_combo),
	gender_combo_(gender_combo),
	color_(0)
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

	if(utils::string_bool(side["random_faction"])) {
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
		if (gender_combo_ != NULL) {
			gender_combo_->enable(false);
			gender_combo_->set_items(genders_);
			gender_combo_->set_selected(0);
		}
		return;
	}

	const unit_type *utp = unit_types.find(leader);
	if (utp) {
		const unit_type &ut = *utp;
		const std::vector<unit_race::GENDER> genders = ut.genders();
		if ( (genders.size() < 2) && (gender_combo_ != NULL) ) {
			gender_combo_->enable(false);
		} else {
			gender_ids_.push_back("random");
			genders_.push_back(IMAGE_PREFIX + random_enemy_picture + COLUMN_SEPARATOR + _("gender^Random"));
			if (gender_combo_ != NULL) gender_combo_->enable(true);
		}
		for (std::vector<unit_race::GENDER>::const_iterator i=genders.begin(); i != genders.end(); ++i) {
			const unit_type& utg = ut.get_gender_unit_type(*i);

			// Make the internationalized titles for each gender, along with the WML ids
			if (*i == unit_race::FEMALE) {
				gender_ids_.push_back("female");
				genders_.push_back(IMAGE_PREFIX + utg.image() + get_RC_suffix(utg.flag_rgb()) +
						COLUMN_SEPARATOR + _("Female ♀"));
			} else {
				gender_ids_.push_back("male");
				genders_.push_back(IMAGE_PREFIX + utg.image() + get_RC_suffix(utg.flag_rgb()) +
						COLUMN_SEPARATOR + _("Male ♂"));
			}
		}
		if (gender_combo_ != NULL) {
			gender_combo_->set_items(genders_);
			assert(!genders_.empty());
			gender_index %= genders_.size();
			gender_combo_->set_selected(gender_index);
		}
	} else {
		gender_ids_.push_back("random");
		genders_.push_back(IMAGE_PREFIX + random_enemy_picture + COLUMN_SEPARATOR + _("Random"));
		if (gender_combo_ != NULL) {
			gender_combo_->enable(false);
			gender_combo_->set_items(genders_);
			gender_combo_->set_selected(0);
		}
	}
}

void leader_list_manager::populate_leader_combo(int selected_index) {
	std::vector<std::string>::const_iterator itor;
	std::vector<std::string> leader_strings;
	for(itor = leaders_.begin(); itor != leaders_.end(); ++itor) {

		const unit_type *utp = unit_types.find(*itor);
		if (utp) {
			std::string gender;
			if (gender_combo_ != NULL && !genders_.empty() && size_t(gender_combo_->selected()) < genders_.size()) {
				gender = gender_ids_[gender_combo_->selected()];
			}
			const unit_type& ut = utp->get_gender_unit_type(gender);
			leader_strings.push_back(IMAGE_PREFIX + ut.image() + get_RC_suffix(ut.flag_rgb()) + COLUMN_SEPARATOR + ut.type_name());

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

#ifdef LOW_MEM
std::string leader_list_manager::get_RC_suffix(const std::string& /*unit_color*/) const {
	return "";
}
#else
std::string leader_list_manager::get_RC_suffix(const std::string& unit_color) const {
	return "~RC("+unit_color+">"+lexical_cast<std::string>(color_+1) +")";
}
#endif
