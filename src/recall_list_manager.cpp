/*
	Copyright (C) 2014 - 2024
	by Chris Beck <render787@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "recall_list_manager.hpp"
#include "units/unit.hpp"
#include "units/ptr.hpp"
#include "utils/general.hpp"

#include <algorithm>
#include <string>
#include <vector>


/**
 * Used to find units in vectors by their ID.
 */
unit_ptr recall_list_manager::find_if_matches_id(const std::string &unit_id)
{
	std::vector<unit_ptr >::iterator it = std::find_if(recall_list_.begin(), recall_list_.end(),
		[&unit_id](const unit_ptr & ptr) { return ptr->id() == unit_id; });
	if (it != recall_list_.end()) {
		return *it;
	} else {
		return unit_ptr();
	}
}

/**
 * Used to find units in vectors by their ID.
 */
unit_const_ptr recall_list_manager::find_if_matches_id(const std::string &unit_id) const
{
	std::vector<unit_ptr >::const_iterator it = std::find_if(recall_list_.begin(), recall_list_.end(),
		[&unit_id](const unit_ptr & ptr) { return ptr->id() == unit_id; });
	if (it != recall_list_.end()) {
		return *it;
	} else {
		return unit_ptr();
	}
}

/**
 * Used to erase units from vectors by their ID.
 */
void recall_list_manager::erase_if_matches_id(const std::string &unit_id)
{
	// using unit_id as reference has potential to cause a crash if the underlying unit becomes invald
	// https://github.com/wesnoth/wesnoth/issues/6603
	utils::erase_if(recall_list_, [unit_id](const unit_ptr& ptr) { return ptr->id() == unit_id; });
}

void recall_list_manager::add(const unit_ptr & ptr, int pos)
{
	if (pos < 0 || pos >= static_cast<int>(recall_list_.size())) {
		recall_list_.push_back(ptr);
	}
	else {
		recall_list_.insert(recall_list_.begin() + pos, ptr);
	}
}

std::size_t recall_list_manager::find_index(const std::string & unit_id) const
{
	std::vector<unit_ptr >::const_iterator it = std::find_if(recall_list_.begin(), recall_list_.end(),
		[&unit_id](const unit_ptr & ptr) { return ptr->id() == unit_id; });

	return std::distance(recall_list_.begin(), it);
}

unit_ptr recall_list_manager::extract_if_matches_id(const std::string &unit_id, int * pos)
{
	std::vector<unit_ptr >::iterator it = std::find_if(recall_list_.begin(), recall_list_.end(),
		[&unit_id](const unit_ptr & ptr) { return ptr->id() == unit_id; });
	if (it != recall_list_.end()) {
		unit_ptr ret = *it;
		if(pos) {
			*pos = it - recall_list_.begin();
		}
		recall_list_.erase(it);
		return ret;
	} else {
		return unit_ptr();
	}
}

unit_ptr recall_list_manager::find_if_matches_underlying_id(std::size_t uid)
{
	std::vector<unit_ptr >::iterator it = std::find_if(recall_list_.begin(), recall_list_.end(),
		[uid](const unit_ptr & ptr) { return ptr->underlying_id() == uid; });
	if (it != recall_list_.end()) {
		return *it;
	} else {
		return unit_ptr();
	}
}

unit_const_ptr recall_list_manager::find_if_matches_underlying_id(std::size_t uid) const
{
	std::vector<unit_ptr >::const_iterator it = std::find_if(recall_list_.begin(), recall_list_.end(),
		[uid](const unit_ptr & ptr) { return ptr->underlying_id() == uid; });
	if (it != recall_list_.end()) {
		return *it;
	} else {
		return unit_ptr();
	}
}

void recall_list_manager::erase_by_underlying_id(std::size_t uid)
{
	utils::erase_if(recall_list_, [uid](const unit_ptr& ptr) { return ptr->underlying_id() == uid; });
}

unit_ptr recall_list_manager::extract_if_matches_underlying_id(std::size_t uid)
{
	std::vector<unit_ptr >::iterator it = std::find_if(recall_list_.begin(), recall_list_.end(),
		[uid](const unit_ptr & ptr) { return ptr->underlying_id() == uid; });
	if (it != recall_list_.end()) {
		unit_ptr ret = *it;
		recall_list_.erase(it);
		return ret;
	} else {
		return unit_ptr();
	}
}

std::vector<unit_ptr>::iterator recall_list_manager::erase_index(std::size_t idx) {
	assert(idx < recall_list_.size());
	return recall_list_.erase(recall_list_.begin()+idx);
}

std::vector<unit_ptr>::iterator recall_list_manager::erase(const std::vector<unit_ptr>::iterator& it) {
	return recall_list_.erase(it);
}
