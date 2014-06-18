/*
   Copyright (C) 2014 by Chris Beck <render787@gmail.com>
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
#include "unit.hpp"
#include "unit_ptr.hpp"

#include <algorithm>
#include <string>
#include <vector>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

bool find_if_matches_helper(const UnitPtr & ptr, const std::string & unit_id)
{
	return ptr->matches_id(unit_id);
}

/**
 * Used to find units in vectors by their ID.
 */
UnitPtr recall_list_manager::find_if_matches_id(const std::string &unit_id)
{
	std::vector<UnitPtr >::iterator it = std::find_if(recall_list_.begin(), recall_list_.end(),
	                    boost::bind(&find_if_matches_helper, _1, unit_id));
	if (it != recall_list_.end()) {
		return *it;
	} else {
		return UnitPtr();
	}
}

/**
 * Used to find units in vectors by their ID.
 */
UnitConstPtr recall_list_manager::find_if_matches_id(const std::string &unit_id) const
{
	std::vector<UnitPtr >::const_iterator it = std::find_if(recall_list_.begin(), recall_list_.end(),
	                    boost::bind(&find_if_matches_helper, _1, unit_id));
	if (it != recall_list_.end()) {
		return *it;
	} else {
		return UnitPtr();
	}
}

/**
 * Used to erase units from vectors by their ID.
 */
void recall_list_manager::erase_if_matches_id(const std::string &unit_id)
{
	recall_list_.erase(std::remove_if(recall_list_.begin(), recall_list_.end(),
	                                  boost::bind(&find_if_matches_helper, _1, unit_id)),
	                       recall_list_.end());
}

void recall_list_manager::add (const UnitPtr & ptr)
{
	recall_list_.push_back(ptr);
}

size_t recall_list_manager::find_index(const std::string & unit_id) const
{
	std::vector<UnitPtr >::const_iterator it = std::find_if(recall_list_.begin(), recall_list_.end(),
	                    boost::bind(&find_if_matches_helper, _1, unit_id));

	return it - recall_list_.begin();
}

UnitPtr recall_list_manager::extract_if_matches_id(const std::string &unit_id)
{
	std::vector<UnitPtr >::iterator it = std::find_if(recall_list_.begin(), recall_list_.end(),
	                    boost::bind(&find_if_matches_helper, _1, unit_id));
	if (it != recall_list_.end()) {
		UnitPtr ret = *it;
		recall_list_.erase(it);
		return ret;
	} else {
		return UnitPtr();
	}
}

bool find_if_matches_uid_helper(const UnitPtr & ptr, size_t uid)
{
	return ptr->underlying_id() == uid;
}

UnitPtr recall_list_manager::find_if_matches_underlying_id(size_t uid)
{
	std::vector<UnitPtr >::iterator it = std::find_if(recall_list_.begin(), recall_list_.end(),
	                    boost::bind(&find_if_matches_uid_helper, _1, uid));
	if (it != recall_list_.end()) {
		return *it;
	} else {
		return UnitPtr();
	}
}

UnitConstPtr recall_list_manager::find_if_matches_underlying_id(size_t uid) const
{
	std::vector<UnitPtr >::const_iterator it = std::find_if(recall_list_.begin(), recall_list_.end(),
	                    boost::bind(&find_if_matches_uid_helper, _1, uid));
	if (it != recall_list_.end()) {
		return *it;
	} else {
		return UnitPtr();
	}
}

void recall_list_manager::erase_by_underlying_id(size_t uid)
{
	recall_list_.erase(std::remove_if(recall_list_.begin(), recall_list_.end(),
	                                  boost::bind(&find_if_matches_uid_helper, _1, uid)),
	                       recall_list_.end());
}

UnitPtr recall_list_manager::extract_if_matches_underlying_id(size_t uid)
{
	std::vector<UnitPtr >::iterator it = std::find_if(recall_list_.begin(), recall_list_.end(),
	                    boost::bind(&find_if_matches_uid_helper, _1, uid));
	if (it != recall_list_.end()) {
		UnitPtr ret = *it;
		recall_list_.erase(it);
		return ret;
	} else {
		return UnitPtr();
	}
}

std::vector<UnitPtr>::iterator recall_list_manager::erase_index(size_t idx) {
	assert(idx < recall_list_.size());
	return recall_list_.erase(recall_list_.begin()+idx);
}

std::vector<UnitPtr>::iterator recall_list_manager::erase(std::vector<UnitPtr>::iterator it) {
	return recall_list_.erase(it);
}
