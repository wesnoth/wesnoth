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

// This class encapsulates the recall list of a team

#ifndef RECALL_LIST_MGR_HPP
#define RECALL_LIST_MGR_HPP

#include "unit_ptr.hpp"

#include <string>
#include <vector>

namespace ai {
	class readonly_context_impl;
}

class recall_list_manager {
public:
	typedef std::vector<UnitPtr >::iterator iterator;
	typedef std::vector<UnitPtr >::const_iterator const_iterator;

	iterator begin() { return recall_list_.begin();}
	iterator end() { return recall_list_.end(); }

	const_iterator begin() const { return recall_list_.begin();}
	const_iterator end() const { return recall_list_.end(); }

	UnitPtr operator[](size_t index) { return recall_list_[index]; }
	UnitConstPtr operator[](size_t index) const { return recall_list_[index]; }

	UnitPtr find_if_matches_id(const std::string & unit_id);
	UnitPtr extract_if_matches_id(const std::string & unit_id);
	UnitConstPtr find_if_matches_id(const std::string & unit_id) const;
	void erase_if_matches_id(const std::string & unit_id);

	UnitPtr find_if_matches_underlying_id(size_t uid);
	UnitPtr extract_if_matches_underlying_id(size_t uid);
	UnitConstPtr find_if_matches_underlying_id(size_t uid) const;
	void erase_by_underlying_id(size_t uid);

	iterator erase_index(size_t index);
	iterator erase(iterator it);

	size_t find_index(const std::string & unit_id) const;
	size_t size() const { return recall_list_.size(); }
	bool empty() const { return recall_list_.empty(); }

	void add(const UnitPtr & ptr);

private:
	std::vector<UnitPtr > recall_list_;

	friend class ai::readonly_context_impl;
};

#endif
