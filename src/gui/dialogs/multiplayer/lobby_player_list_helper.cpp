/*
	Copyright (C) 2009 - 2024
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

#include "gui/dialogs/multiplayer/lobby_player_list_helper.hpp"

#include "serialization/markup.hpp"
#include "game_initialization/lobby_data.hpp"
#include "gettext.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/toggle_panel.hpp"
#include "gui/widgets/tree_view.hpp"
#include "gui/widgets/window.hpp"

static lg::log_domain log_lobby("lobby");
#define ERR_LB LOG_STREAM(err, log_lobby)

namespace gui2
{
lobby_player_list_helper::sub_list::sub_list(tree_view* parent_tree, const std::string& lbl, const bool unfolded)
{
	widget_data tree_group_item;
	tree_group_item["tree_view_node_label"]["label"] = lbl;

	root = &parent_tree->add_node("player_group", tree_group_item);

	if(unfolded) {
		root->unfold();
	}

	label_player_count = root->find_widget<label>("player_count", false, true);
	assert(label_player_count);
}

void lobby_player_list_helper::sub_list::update_player_count_label()
{
	label_player_count->set_label(std::to_string(root->count_children()));
}

namespace
{
struct update_pod
{
	/** The raw data used to mass-construct player tree nodes. */
	std::vector<widget_data> node_data;

	/** The associated user data for each node, index-to-index. */
	std::vector<const mp::user_info*> user_data;
};
} // namespace

void lobby_player_list_helper::update(const std::vector<mp::user_info>& user_info, int focused_game)
{
	const unsigned scrollbar_position = tree->get_vertical_scrollbar_item_position();
	std::array<update_pod, std::tuple_size<decltype(player_lists)>::value> inputs{};

	for(const auto& user : user_info) {
		std::string name = user.name;

		std::stringstream icon_ss;
		icon_ss << "lobby/status";

		switch(user.get_state(focused_game)) {
		case mp::user_info::user_state::LOBBY:
			icon_ss << "-lobby";
			break;
		case mp::user_info::user_state::SEL_GAME:
			name = markup::span_color(color_t(0, 255, 255), name);
			icon_ss << (user.observing ? "-obs" : "-playing");
			break;
		case mp::user_info::user_state::GAME:
			name = markup::span_color(font::GRAY_COLOR, name);
			icon_ss << (user.observing ? "-obs" : "-playing");
			break;
		}

		switch(user.get_relation()) {
		case mp::user_info::user_relation::ME:
			icon_ss << "-s";
			break;
		case mp::user_info::user_relation::NEUTRAL:
			icon_ss << "-n";
			break;
		case mp::user_info::user_relation::FRIEND:
			icon_ss << "-f";
			break;
		case mp::user_info::user_relation::IGNORED:
			icon_ss << "-i";
			break;
		}

		icon_ss << ".png";

		widget_item tree_group_field;
		widget_data tree_group_item;

		/*** Add tree item ***/
		tree_group_field["label"] = icon_ss.str();
		tree_group_item["icon"] = tree_group_field;

		tree_group_field["label"] = name;
		tree_group_field["use_markup"] = "true";
		tree_group_item["name"] = tree_group_field;

		// Indices here must match the order of the lists in the player_lists array (see `init`)
		switch(user.get_state(focused_game)) {
		case mp::user_info::user_state::SEL_GAME:
			inputs[0].node_data.push_back(std::move(tree_group_item));
			inputs[0].user_data.push_back(&user);

			break;
		case mp::user_info::user_state::LOBBY:
			inputs[1].node_data.push_back(std::move(tree_group_item));
			inputs[1].user_data.push_back(&user);

			break;
		case mp::user_info::user_state::GAME:
			inputs[2].node_data.push_back(std::move(tree_group_item));
			inputs[2].user_data.push_back(&user);

			break;
		}
	}

	info_map.clear();

	for(std::size_t i = 0; i < player_lists.size(); ++i) {
		assert(inputs[i].node_data.size() == inputs[i].user_data.size());

		// Add the player nodes
		const auto new_nodes = player_lists[i].root->replace_children("player", inputs[i].node_data);

		for(std::size_t k = 0; k < new_nodes.size(); ++k) {
			auto* node = new_nodes[k].get();
			auto* info = inputs[i].user_data[k];

			// Note the user_info associated with this node
			info_map.try_emplace(node, info);

			connect_signal_mouse_left_double_click(
				node->find_widget<toggle_panel>("tree_view_node_label"),
				std::bind(user_callback, info)
			);
		}

		player_lists[i].update_player_count_label();
	}

	// Don't attempt to restore the scroll position if the window hasn't been laid out yet
	if(tree->get_origin() != point{-1, -1}) {
		tree->set_vertical_scrollbar_item_position(scrollbar_position);
	}
}

void lobby_player_list_helper::init(window& w)
{
	tree = w.find_widget<tree_view>("player_tree", false, true);

	player_lists = {
		sub_list{tree, _("Selected Game"), true},
		sub_list{tree, _("Lobby"), true},
		sub_list{tree, _("Other Games"), false}
	};
}

const mp::user_info* lobby_player_list_helper::get_selected_info() const
{
	return info_map.at(tree->selected_item());
}

} // namespace gui2
