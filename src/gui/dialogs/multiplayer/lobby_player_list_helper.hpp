/*
	Copyright (C) 2009 - 2025
	by Tomasz Sniatowski <kailoran@gmail.com>
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

#include <array>
#include <functional>
#include <map>
#include <string>

namespace mp
{
struct user_info;
}

namespace gui2
{
class label;
class tree_view;
class tree_view_node;
class window;

class lobby_player_list_helper
{
public:
	void init(window& w);

	/** Updates the tree contents based on the given user data. */
	void update(const std::vector<mp::user_info>& user_info, int focused_game);

private:
	struct sub_list
	{
		sub_list() = default;
		sub_list(tree_view* parent_tree, const std::string& label, const bool unfolded);

		void update_player_count_label();

		tree_view_node* root;

		label* label_player_count;
	};

	/** Main subsections of the player tree. This should never change at runtime so the number is hardcoded. */
	std::array<sub_list, 3> player_lists;

	/** The parent tree. */
	tree_view* tree;

	/** The double click callback bound to each player's tree node. */
	std::function<void(const mp::user_info*)> user_callback;

	/** Node-to-info mappings for easy access. */
	std::map<const tree_view_node*, const mp::user_info*> info_map;

public:
	lobby_player_list_helper(decltype(user_callback) ucb)
		: player_lists()
		, tree(nullptr)
		, user_callback(ucb)
		, info_map()
	{
	}

	const mp::user_info* get_selected_info() const;
};
} // namespace gui2
