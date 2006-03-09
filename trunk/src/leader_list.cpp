/* $Id$ */
/*
   Copyright (C)
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
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
		const game_data* data, gui::combo* combo) :
	side_list_(side_list), data_(data), combo_(combo)
{
}

void leader_list_manager::set_combo(gui::combo* combo)
{
	combo_ = combo;

	if (combo_ != NULL) {
		update_leader_list(0);
	}
}

void leader_list_manager::update_leader_list(int side_index)
{
	const config& side = *side_list_[side_index];

	leaders_.clear();

	if(side["random_faction"] == "yes") {
		if(combo_ != NULL) {
			std::vector<std::string> dummy;
			dummy.push_back("-");
			combo_->enable(false);
			combo_->set_items(dummy);
			combo_->set_selected(0);
		}
		return;
	} else {
		if(combo_ != NULL)
			combo_->enable(true);
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

	std::vector<std::string> leader_strings;

	for(itor = leaders_.begin(); itor != leaders_.end(); ++itor) {

		const game_data::unit_type_map& utypes = data_->unit_types;

		//const std::string name = data_->unit_types->find(*itor).language_name();
		if (utypes.find(*itor) != utypes.end()) {
			const std::string name =  utypes.find(*itor)->second.language_name();
			const std::string image = utypes.find(*itor)->second.image();

			leader_strings.push_back(IMAGE_PREFIX + image + COLUMN_SEPARATOR + name);
		} else {
			leader_strings.push_back("?");
		}
	}

	leaders_.push_back("random");
	leader_strings.push_back(IMAGE_PREFIX + random_enemy_picture +
	                         COLUMN_SEPARATOR + _("Random"));

	if(combo_ != NULL) {
		combo_->set_items(leader_strings);
		combo_->set_selected(default_index);
	}
}

void leader_list_manager::set_leader(const std::string& leader)
{
	if(combo_ == NULL)
		return;

	int leader_index = 0;
	for(std::vector<std::string>::const_iterator itor = leaders_.begin();
			itor != leaders_.end(); ++itor) {
		if (leader == *itor)
			combo_->set_selected(leader_index);
		++leader_index;
	}
}

std::string leader_list_manager::get_leader() const
{
	if(combo_ == NULL)
		return _("?");

	if(leaders_.empty())
		return "random";

	if(size_t(combo_->selected()) >= leaders_.size())
		return _("?");

	return leaders_[combo_->selected()];
}

