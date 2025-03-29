/*
	Copyright (C) 2017 - 2025
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

#include "gui/widgets/button.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/rich_label.hpp"
#include "gui/widgets/scroll_label.hpp"
#include "gui/widgets/scrollbar_panel.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/tree_view.hpp"
#include "gui/widgets/tree_view_node.hpp"
#include "gui/widgets/window.hpp"
#include "help/help.hpp"
#include "serialization/string_utils.hpp"
#include "utils/ci_searcher.hpp"
#include "video.hpp"

static lg::log_domain log_help("help");
#define ERR_HP LOG_STREAM(err, log_help)
#define WRN_HP LOG_STREAM(warn, log_help)
#define DBG_HP LOG_STREAM(debug, log_help)

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

void help_browser::pre_show()
{
	tree_view& topic_tree = find_widget<tree_view>("topic_tree");

	button& back_button = find_widget<button>("back");
	button& next_button = find_widget<button>("next");

	rich_label& topic_text = find_widget<rich_label>("topic_text");

	next_button.set_active(false);
	back_button.set_active(false);
	connect_signal_mouse_left_click(back_button, std::bind(&help_browser::on_history_navigate, this, true));
	connect_signal_mouse_left_click(next_button, std::bind(&help_browser::on_history_navigate, this, false));

	toggle_button& contents = find_widget<toggle_button>("contents");

	if (video::window_size().x <= 800) {
		contents.set_value(false);
		connect_signal_mouse_left_click(contents, [&](auto&&...) {
			topic_tree.set_visible(topic_tree.get_visible() == widget::visibility::visible
				? widget::visibility::invisible
				: widget::visibility::visible);
			invalidate_layout();
		});
		topic_tree.set_visible(widget::visibility::invisible);
	} else {
		contents.set_value(true);
		contents.set_visible(widget::visibility::invisible);
	}

	text_box& filter = find_widget<text_box>("filter_box");
	add_to_keyboard_chain(&filter);
	filter.on_modified([this](const auto& box) { update_list(box.text()); });

	topic_text.register_link_callback(std::bind(&help_browser::show_topic, this, std::placeholders::_1, true));

	connect_signal_notify_modified(topic_tree, std::bind(&help_browser::on_topic_select, this));

	keyboard_capture(&topic_tree);

	add_topics_for_section(toplevel_, topic_tree.get_root_node());

	tree_view_node& initial_node = topic_tree.find_widget<tree_view_node>(initial_topic_);
	initial_node.select_node(true);

	on_topic_select();
}

void help_browser::update_list(const std::string& filter_text) {
	tree_view& topic_tree = find_widget<tree_view>("topic_tree");
	topic_tree.clear();
	add_topics_for_section(toplevel_, topic_tree.get_root_node(), filter_text);
}

bool help_browser::add_topics_for_section(const help::section& parent_section, tree_view_node& parent_node, const std::string& filter_text)
{
	bool topics_added = false;
	const auto match = translation::make_ci_matcher(filter_text);

	for(const help::section& section : parent_section.sections) {
		tree_view_node& section_node = add_topic(section.id, section.title, true, parent_node);
		bool subtopics_added = add_topics_for_section(section, section_node, filter_text);

		if (subtopics_added || match(section.id)) {
			if (!filter_text.empty()) {
				section_node.unfold();
			}
			topics_added = true;
		} else {
			find_widget<tree_view>("topic_tree").remove_node(&section_node);
		}
	}

	for(const help::topic& topic : parent_section.topics) {
		if ((match(topic.id) || match(topic.title)) && (topic.id.compare(0, 2, "..") != 0)) {
			add_topic(topic.id, topic.title, false, parent_node);
			topics_added = true;
		}
	}

	return topics_added;
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
			ERR_HP << "Help browser tried to show topic with id '" << topic_id
				  << "' but that topic could not be found." << std::endl;
			return;
		}

		DBG_HP << "Showing topic: " << topic->id << ": " << topic->title;

		widget_data data;
		widget_item item;

		item["label"] = topic->title;
		data.emplace("topic_title", item);

		find_widget<label>("topic_title").set_label(topic->title);
		find_widget<rich_label>("topic_text").set_dom(topic->text.parsed_text());

		invalidate_layout();
		scrollbar_panel& scroll = find_widget<scrollbar_panel>("topic_scroll_panel");
		scroll.scroll_vertical_scrollbar(scrollbar_base::BEGIN);
	}

	if (add_to_history) {
		// history pos is 0 initially, so it's already at first entry
		// no need increment first time
		if (!history_.empty()) {
			history_pos_++;
		}
		history_.push_back(topic_id);

		find_widget<button>("back").set_active(history_pos_ != 0);
	}
}

void help_browser::on_topic_select()
{
	tree_view& topic_tree = find_widget<tree_view>("topic_tree");

	if(topic_tree.empty()) {
		return;
	}

	tree_view_node* selected = topic_tree.selected_item();
	assert(selected);

	if (selected->id()[0] != '+' && video::window_size().x <= 800) {
		find_widget<toggle_button>("contents").set_value(false);
		topic_tree.set_visible(widget::visibility::invisible);
	}

	show_topic(selected->id());
}

void help_browser::on_history_navigate(bool backwards)
{
	if(backwards) {
		history_pos_--;
	} else {
		history_pos_++;
	}
	find_widget<button>("back").set_active(!history_.empty() && history_pos_ != 0);
	find_widget<button>("next").set_active(!history_.empty() && history_pos_ != (history_.size()-1));

	const std::string topic_id = history_.at(history_pos_);
	show_topic(topic_id, false);
}

} // namespace dialogs
