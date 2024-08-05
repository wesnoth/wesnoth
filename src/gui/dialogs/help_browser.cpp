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

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/help_browser.hpp"

#include "game_config_manager.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/rich_label.hpp"
#include "gui/widgets/scroll_label.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/tree_view.hpp"
#include "gui/widgets/tree_view_node.hpp"
#include "gui/widgets/window.hpp"

#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif

#include "help/help.hpp"
#include "help/help_impl.hpp"
#include "font/pango/escape.hpp"
#include "font/pango/hyperlink.hpp"

namespace gui2::dialogs
{

REGISTER_DIALOG(help_browser)

help_browser::help_browser(const help::section& toplevel, const std::string& initial)
	: modal_dialog(window_id())
	, initial_topic_(initial.empty() ? help::default_show_topic : initial)
	, toplevel_(toplevel)
	, history_()
	, history_pos_(0)
{
	if(initial_topic_.compare(0, 2, "..") == 0) {
		initial_topic_.replace(0, 2, "+");
	} else {
		initial_topic_.insert(0, "-");
	}
	help::init_help();
}

void help_browser::pre_show(window& window)
{
	tree_view& topic_tree = find_widget<tree_view>(&window, "topic_tree", false);

	button& back_button = find_widget<button>(&window, "back", false);
	button& next_button = find_widget<button>(&window, "next", false);

	rich_label& topic_text = find_widget<rich_label>(&window, "topic_text", false);

	next_button.set_active(false);
	back_button.set_active(false);
	connect_signal_mouse_left_click(back_button, std::bind(&help_browser::on_history_navigate, this, true));
	connect_signal_mouse_left_click(next_button, std::bind(&help_browser::on_history_navigate, this, false));

	topic_text.register_link_callback(std::bind(&help_browser::on_link_click, this, std::placeholders::_1));

	connect_signal_notify_modified(topic_tree, std::bind(&help_browser::on_topic_select, this));

	window.keyboard_capture(&topic_tree);

	add_topics_for_section(toplevel_, topic_tree.get_root_node());

	tree_view_node& initial_node = find_widget<tree_view_node>(&topic_tree, initial_topic_, false);
	initial_node.select_node(true);

	on_topic_select();
}

void help_browser::add_topics_for_section(const help::section& parent_section, tree_view_node& parent_node)
{
	for(const help::section& section : parent_section.sections) {
		tree_view_node& section_node = add_topic(section.id, section.title, true, parent_node);

		add_topics_for_section(section, section_node);
	}

	for(const help::topic& topic : parent_section.topics) {
		if(topic.id.compare(0, 2, "..") != 0) {
			add_topic(topic.id, topic.title, false, parent_node);
		}
	}
}

tree_view_node& help_browser::add_topic(const std::string& topic_id, const std::string& topic_title,
		bool expands, tree_view_node& parent)
{
	widget_data data;
	widget_item item;

	item["label"] = topic_title;
	data.emplace("topic_name", item);

	tree_view_node& new_node = parent.add_child(expands ? "section" : "topic", data);
	new_node.set_id(std::string(expands ? "+" : "-") + topic_id);

	return new_node;
}
void help_browser::show_topic(std::string topic_id, bool add_to_history)
{
	if(topic_id.empty()) {
		return;
	}

	if(topic_id[0] == '+') {
		topic_id.replace(topic_id.begin(), topic_id.begin() + 1, 2, '.');
	}

	if(topic_id[0] == '-') {
		topic_id.erase(topic_id.begin(), topic_id.begin() + 1);
	}

	auto iter = parsed_pages_.find(topic_id);
	if(iter == parsed_pages_.end()) {
		const help::topic* topic = help::find_topic(toplevel_, topic_id);
		if(!topic) {
			ERR_GUI_P << "Help browser tried to show topic with id '" << topic_id
				  << "' but that topic could not be found." << std::endl;
			return;
		}

		widget_data data;
		widget_item item;

		item["label"] = topic->title;
		data.emplace("topic_title", item);

		find_widget<label>(this, "topic_title", false).set_label(topic->title);
		find_widget<rich_label>(this, "topic_text", false).set_topic(topic);

		get_window()->invalidate_layout();
	}

	if (add_to_history) {
		// history pos is 0 initially, so it's already at first entry
		// no need increment first time
		if (!history_.empty()) {
			history_pos_++;
		}
		history_.push_back(topic_id);

		find_widget<button>(this, "back", false).set_active(history_pos_ != 0);

	}
}

void help_browser::on_link_click(std::string link)
{
	show_topic(link);
}

void help_browser::on_topic_select()
{
	tree_view& topic_tree = find_widget<tree_view>(this, "topic_tree", false);

	if(topic_tree.empty()) {
		return;
	}

	tree_view_node* selected = topic_tree.selected_item();
	assert(selected);

	show_topic(selected->id());
}

void help_browser::on_history_navigate(bool backwards)
{
	if(backwards) {
		history_pos_--;
	} else {
		history_pos_++;
	}
	find_widget<button>(this, "back", false).set_active(!history_.empty() && history_pos_ != 0);
	find_widget<button>(this, "next", false).set_active(!history_.empty() && history_pos_ != (history_.size()-1));

	PLAIN_LOG << "history pos: " << history_pos_;
	const std::string topic_id = history_.at(history_pos_);
	PLAIN_LOG << " history: " << topic_id;
	show_topic(topic_id, false);
//	const unsigned topic_i = parsed_pages_.at(*history_pos_);
//	find_widget<multi_page>(this, "topic_text_pages", false).select_page(topic_i);
}

} // namespace dialogs
