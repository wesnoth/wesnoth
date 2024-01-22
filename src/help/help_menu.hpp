/*
	Copyright (C) 2003 - 2024
	by David White <dave@whitevine.net>
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

#include <set>                          // for set
#include <string>                       // for string, basic_string
#include <vector>                       // for vector
#include "widgets/menu.hpp"             // for menu

namespace help { struct section; }
namespace help { struct topic; }

namespace help {

/**
 * The menu to the left in the help browser, where topics can be
 * navigated through and chosen.
 */
class help_menu : public gui::menu
{
public:
	help_menu(const section &toplevel, int max_height=-1);
	int process();

	/**
	 * Make the topic the currently selected one, and expand all
	 * sections that need to be expanded to show it.
	 */
	void select_topic(const topic &t);

	/**
	 * If a topic has been chosen, return that topic, otherwise
	 * nullptr. If one topic is returned, it will not be returned again,
	 * if it is not re-chosen.
	 */
	const topic *chosen_topic();

private:
	/** Information about an item that is visible in the menu. */
	struct visible_item {
		visible_item(const section *_sec, int level);
		visible_item(const topic *_t, int level);
		// Invariant, one if these should be nullptr. The constructors
		// enforce it.
		const topic *t;
		const section *sec;
		gui::indented_menu_item get_menu_item(const help_menu& parent) const;
		/** Indentation level, always one more than the parent section. */
		int level;
		bool operator==(const visible_item &sec) const;
		bool operator==(const section &sec) const;
		bool operator==(const topic &t) const;
	};

	/** Regenerate what items are visible by checking what sections are expanded. */
	void update_visible_items(const section &top_level, unsigned starting_level=0);

	/** Return true if the section is expanded. */
	bool expanded(const section &sec) const;

	/** Mark a section as expanded. Do not update the visible items or anything. */
	void expand(const section &sec);

	/**
	 * Contract (close) a section. That is, mark it as not expanded,
	 * visible items are not updated.
	 */
	void contract(const section &sec);

	/**
	 * Return the string to use as the prefix for the icon part of the
	 * menu-string at the specified level.
	 */
	std::string indent_list(const std::string &icon, const unsigned level);
	/** Return the data to use with the superclass's set_items() for sections at the specified level. */
	gui::indented_menu_item get_item_to_show(const section &sec, const unsigned level);
	/** Return the data to use with the superclass's set_items() for topics at the specified level. */
	gui::indented_menu_item get_item_to_show(const topic &topic, const unsigned level);

	/** Draw the currently visible items. */
	void display_visible_items();

	/**
	 * Internal recursive thingie. did_expand will be true if any
	 * section was expanded, otherwise untouched.
	 */
	bool select_topic_internal(const topic &t, const section &sec);

	std::vector<visible_item> visible_items_;
	const section &toplevel_;
	std::set<const section*> expanded_;
	topic const *chosen_topic_;
	visible_item selected_item_;
};

} // end namespace help
