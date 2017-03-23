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

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/help_browser.hpp"

#include "game_config_manager.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/multi_page.hpp"
#include "gui/widgets/scroll_label.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/tree_view.hpp"
#include "gui/widgets/window.hpp"

#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif

#include "help/help.hpp"

namespace gui2
{
namespace dialogs
{

REGISTER_DIALOG(help_browser)

help_browser::help_browser()
	: initial_topic_("introduction")
    , help_cfg_(game_config_manager::get()->game_config().child("help"))
{
}

void help_browser::pre_show(window& window)
{
    tree_view& topic_tree = find_widget<tree_view>(&window, "topic_tree", false);

	for(const auto& topic : help_cfg_.child_range("topic")) {
		std::map<std::string, string_map> data;
		string_map item;

        item["label"] = topic["title"];
        data.emplace("topic_name", item);

        topic_tree.add_node("topic", data);
    }
}

} // namespace dialogs
} // namespace gui2
