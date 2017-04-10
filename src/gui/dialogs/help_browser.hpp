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
#include <map>

class config;

namespace gui2::dialogs
{

/** Help browser dialog. */
class help_browser : public modal_dialog
{
public:
	help_browser();

	DEFINE_SIMPLE_DISPLAY_WRAPPER(help_browser)

private:
	std::string initial_topic_;

	std::map<std::string, int> parsed_pages_;

	virtual const std::string& window_id() const override;

	virtual void pre_show(window& window) override;

	void on_topic_select();

	void add_topic(const std::string& topic_id, const std::string& topic_title, bool expands, class tree_view_node* parent = nullptr);
};

} // namespace dialogs
