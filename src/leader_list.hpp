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

/** @file leader_list.hpp */

#ifndef LEADER_LIST_HPP_INCLUDED
#define LEADER_LIST_HPP_INCLUDED

class config;
namespace gui { class combo; }

#include "unit_types.hpp"
#include <string>
#include <vector>

class leader_list_manager
{
public:
	static const std::string random_enemy_picture;

	leader_list_manager(const std::vector<const config *> &side_list,
			gui::combo* leader_combo = NULL, gui::combo* gender_combo = NULL);

	void set_leader_combo(gui::combo* combo);
	void set_gender_combo(gui::combo* combo);
	void update_leader_list(int side);
	void update_gender_list(const std::string& leader);
	std::string get_leader() const;
	std::string get_gender() const;
	void set_leader(const std::string& leader);
	void set_gender(const std::string& gender);
	void set_colour(int colour) {colour_ = colour;};
	std::string get_RC_suffix(const std::string& unit_colour) const;

private:
	void populate_leader_combo(int selected_index);
	std::vector<std::string> leaders_;
	std::vector<std::string> genders_;
	std::vector<std::string> gender_ids_;
	std::vector<const config *> side_list_;
	gui::combo* leader_combo_;
	gui::combo* gender_combo_;
	int colour_;
};

#endif

