/*
   Copyright (C) 2014 - 2018 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

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

#include <algorithm>
#include <string>
#include <vector>

#include "utils/functional.hpp"

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
	recall_list_.erase(std::remove_if(recall_list_.begin(), recall_list_.end(),
		[&unit_id](const unit_ptr & ptr) { return ptr->id() == unit_id; }),
	                       recall_list_.end());
}

void recall_list_manager::add (const unit_ptr & ptr)
{
	recall_list_.push_back(ptr);
}

size_t recall_list_manager::find_index(const std::string & unit_id) const
{
	std::vector<unit_ptr >::const_iterator it = std::find_if(recall_list_.begin(), recall_list_.end(),
		[&unit_id](const unit_ptr & ptr) { return ptr->id() == unit_id; });

	return it - recall_list_.begin();
}

unit_ptr recall_list_manager::extract_if_matches_id(const std::string &unit_id)
{
	std::vector<unit_ptr >::iterator it = std::find_if(recall_list_.begin(), recall_list_.end(),
		[&unit_id](const unit_ptr & ptr) { return ptr->id() == unit_id; });
	if (it != recall_list_.end()) {
		unit_ptr ret = *it;
		recall_list_.erase(it);
		return ret;
	} else {
		return unit_ptr();
	}
}

unit_ptr recall_list_manager::find_if_matches_underlying_id(size_t uid)
{
	std::vector<unit_ptr >::iterator it = std::find_if(recall_list_.begin(), recall_list_.end(),
		[uid](const unit_ptr & ptr) { return ptr->underlying_id() == uid; });
	if (it != recall_list_.end()) {
		return *it;
	} else {
		return unit_ptr();
	}
}

unit_const_ptr recall_list_manager::find_if_matches_underlying_id(size_t uid) const
{
	std::vector<unit_ptr >::const_iterator it = std::find_if(recall_list_.begin(), recall_list_.end(),
		[uid](const unit_ptr & ptr) { return ptr->underlying_id() == uid; });
	if (it != recall_list_.end()) {
		return *it;
	} else {
		return unit_ptr();
	}
}

void recall_list_manager::erase_by_underlying_id(size_t uid)
{
	recall_list_.erase(std::remove_if(recall_list_.begin(), recall_list_.end(),
		[uid](const unit_ptr & ptr) { return ptr->underlying_id() == uid; }),
	                       recall_list_.end());
}

unit_ptr recall_list_manager::extract_if_matches_underlying_id(size_t uid)
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

std::vector<unit_ptr>::iterator recall_list_manager::erase_index(size_t idx) {
	assert(idx < recall_list_.size());
	return recall_list_.erase(recall_list_.begin()+idx);
}

std::vector<unit_ptr>::iterator recall_list_manager::erase(std::vector<unit_ptr>::iterator it) {
	return recall_list_.erase(it);
}
