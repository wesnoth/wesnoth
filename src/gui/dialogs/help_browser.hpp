/*
   Copyright (C) 2017 by Charles Dang <exodia339@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

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
#include <list>

class config;
class CVideo;

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

	static void display(CVideo& video, const help::section& toplevel, const std::string& initial = "")
	{
		help_browser(toplevel, initial).show(video);
	}

private:
	std::string initial_topic_;
	const help::section& toplevel_;

	std::map<std::string, int> parsed_pages_;

	std::list<std::string> history_;
	std::list<std::string>::const_iterator history_pos_;

	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const override;

	/** Inherited from modal_dialog. */
	virtual void pre_show(window& window) override;

	void on_topic_select(window& window);
	void on_history_navigate(window& window, bool backwards);

	void add_topics_for_section(const help::section& parent_section, tree_view_node& parent_node);
	tree_view_node& add_topic(const std::string& topic_id, const std::string& topic_title,
			bool expands, tree_view_node& parent);
};

} // namespace dialogs
} // namespace gui2
