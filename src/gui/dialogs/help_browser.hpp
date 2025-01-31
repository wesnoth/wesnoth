/*
	Copyright (C) 2017 - 2024
	by Charles Dang <exodia339@gmail.com>
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

#include "gui/dialogs/modal_dialog.hpp"
#include "help/help_impl.hpp"
#include <map>
#include <vector>

class config;
class game_config_view;

namespace help {
	struct section;
}

namespace gui2
{
class tree_view_node;

namespace dialogs
{

/** Help browser dialog. */
class help_browser : public modal_dialog
{
public:
	help_browser(const help::section& toplevel, const std::string& initial = "");

	DEFINE_SIMPLE_DISPLAY_WRAPPER(help_browser)

private:
	std::string initial_topic_;
	const help::section& toplevel_;

	std::map<std::string, int> parsed_pages_;

	std::vector<std::string> history_;
	unsigned history_pos_;

	virtual const std::string& window_id() const override;

	virtual void pre_show() override;

	void on_topic_select();
	void on_history_navigate(bool backwards);

	void add_topics_for_section(const help::section& parent_section, tree_view_node& parent_node);
	tree_view_node& add_topic(const std::string& topic_id, const std::string& topic_title,
			bool expands, tree_view_node& parent);
	void show_topic(std::string topic_id, bool add_to_history = true);
};

} // namespace dialogs
} // namespace gui2
