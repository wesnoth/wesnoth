/*
   Copyright (C) 2014 - 2017 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/// This class encapsulates the recall list of a team.

#pragma once

#include "units/ptr.hpp"

#include <string>
#include <vector>

namespace ai {
	class readonly_context_impl;
}

class recall_list_manager {
public:
	typedef std::vector<unit_ptr >::iterator iterator;
	typedef std::vector<unit_ptr >::const_iterator const_iterator;

	iterator begin() { return recall_list_.begin();} //!< begin iterator
	iterator end() { return recall_list_.end(); } //!< end iterator

	const_iterator begin() const { return recall_list_.begin();} //!< begin const iterator
	const_iterator end() const { return recall_list_.end(); } //!< end const iterator

	unit_ptr operator[](size_t index) { return recall_list_[index]; } //!< vector style dereference
	unit_const_ptr operator[](size_t index) const { return recall_list_[index]; } //!< vector style dereference

	unit_ptr find_if_matches_id(const std::string & unit_id); //!< Find a unit by id. Null pointer if not found.
	unit_ptr extract_if_matches_id(const std::string & unit_id); //!< Find a unit by id, and extract from this object if found. Null if not found.
	unit_const_ptr find_if_matches_id(const std::string & unit_id) const; //!< Const find by id.
	void erase_if_matches_id(const std::string & unit_id); //!< Erase any unit with this id.

	unit_ptr find_if_matches_underlying_id(size_t uid); //!< Find a unit by underlying id. Null pointer if not found.
	unit_ptr extract_if_matches_underlying_id(size_t uid); //!< Find a unit by underlying id, and extract if found. Null if not found.
	unit_const_ptr find_if_matches_underlying_id(size_t uid) const; //!< Const find by underlying id.
	void erase_by_underlying_id(size_t uid); //!< Erase any unit with this underlying id.

	iterator erase_index(size_t index); //!< Erase by index.
	iterator erase(iterator it); //!< Erase an iterator to this object.

	size_t find_index(const std::string & unit_id) const; //!< Find the index of a unit by its id.
	size_t size() const { return recall_list_.size(); } //!< Get the number of units on the list.
	bool empty() const { return recall_list_.empty(); } //!< Is it empty?

	void add(const unit_ptr & ptr); //!< Add a unit to the list.

private:
	std::vector<unit_ptr > recall_list_; //!< The underlying data struture. TODO: Should this be a map based on underlying id instead?

	friend class ai::readonly_context_impl; //!< Friend AI module for ease of implementation there.
};
