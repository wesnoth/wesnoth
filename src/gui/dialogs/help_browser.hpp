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

#ifndef GUI_DIALOGS_HELP_BROWSER_HPP_INCLUDED
#define GUI_DIALOGS_HELP_BROWSER_HPP_INCLUDED

#include "gui/dialogs/modal_dialog.hpp"

#include "help/help_impl.hpp"

#include <map>

class config;
class CVideo;

namespace gui2
{
class tree_view_node;

namespace dialogs
{

/** Help browser dialog. */
class help_browser : public modal_dialog
{
public:
	help_browser();

	static void display(CVideo& video)
	{
		help_browser().show(video);
	}

private:
	std::string initial_topic_;

	std::map<std::string, int> parsed_pages_;

	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const override;

	/** Inherited from modal_dialog. */
	virtual void pre_show(window& window) override;

	void on_topic_select(window& window);

	void add_topics_for_section(const help::section& parent_section, tree_view_node& parent_node);
	tree_view_node& add_topic(const std::string& topic_id, const std::string& topic_title,
			bool expands, tree_view_node& parent);
};

} // namespace dialogs
} // namespace gui2

#endif
