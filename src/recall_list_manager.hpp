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

#pragma once

#include "units/ptr.hpp"

#include <string>
#include <vector>

/** This class encapsulates the recall list of a team. */
class recall_list_manager {
public:
	typedef std::vector<unit_ptr >::iterator iterator;
	typedef std::vector<unit_ptr >::const_iterator const_iterator;

	/** begin iterator */
	iterator begin() { return recall_list_.begin();}
	/** end iterator */
	iterator end() { return recall_list_.end(); }

	/** begin const iterator */
	const_iterator begin() const { return recall_list_.begin();}
	/** end const iterator */
	const_iterator end() const { return recall_list_.end(); }

	/** vector style dereference */
	unit_ptr operator[](std::size_t index) { return recall_list_[index]; }
	/** vector style dereference */
	unit_const_ptr operator[](std::size_t index) const { return recall_list_[index]; }

	/** Find a unit by id. Null pointer if not found. */
	unit_ptr find_if_matches_id(const std::string & unit_id);
	/**
	 * Find a unit by id, and extract from this object if found. Null if not found.
	 * @a pos an output paramter, to know in which position the unit was.
	 */
	unit_ptr extract_if_matches_id(const std::string & unit_id, int * pos = nullptr);
	/** Const find by id. */
	unit_const_ptr find_if_matches_id(const std::string & unit_id) const;
	/** Erase any unit with this id. */
	void erase_if_matches_id(const std::string & unit_id);

	/** Find a unit by underlying id. Null pointer if not found. */
	unit_ptr find_if_matches_underlying_id(std::size_t uid);
	/** Find a unit by underlying id, and extract if found. Null if not found. */
	unit_ptr extract_if_matches_underlying_id(std::size_t uid);
	/** Const find by underlying id. */
	unit_const_ptr find_if_matches_underlying_id(std::size_t uid) const;
	/** Erase any unit with this underlying id. */
	void erase_by_underlying_id(std::size_t uid);

	/** Erase by index. */
	iterator erase_index(std::size_t index);
	/** Erase an iterator to this object. */
	iterator erase(const iterator& it);

	/** Find the index of a unit by its id. */
	std::size_t find_index(const std::string & unit_id) const;
	/** Get the number of units on the list. */
	std::size_t size() const { return recall_list_.size(); }
	/** Is it empty? */
	bool empty() const { return recall_list_.empty(); }

	/**
	 * Add a unit to the list.
	 * @a pos the location where to insert the unit, -1 for 'at end'
	 */
	void add(const unit_ptr & ptr, int pos = -1);

private:
	/**
	 * The underlying data struture.
	 * TODO: Should this be a map based on underlying id instead?
	 */
	std::vector<unit_ptr > recall_list_;
};
