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

#include "font/pango/escape.hpp"
#include "font/standard_colors.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/rich_label.hpp"
#include "gui/widgets/scroll_label.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/tree_view.hpp"
#include "gui/widgets/tree_view_node.hpp"
#include "gui/widgets/window.hpp"
#include "help/help.hpp"
#include "serialization/markup.hpp"
#include "serialization/string_utils.hpp"
#include "utils/ci_searcher.hpp"

static lg::log_domain log_help("help");
#define ERR_HP LOG_STREAM(err, log_help)
#define WRN_HP LOG_STREAM(warn, log_help)
#define DBG_HP LOG_STREAM(debug, log_help)

namespace gui2::dialogs
{

namespace
{
	int win_w = 0;
	int rl_init_w = 0;
}

REGISTER_DIALOG(help_browser)

help_browser::help_browser(const help::section& toplevel, const std::string& initial)
	: modal_dialog(window_id())
	, initial_topic_(initial.empty() ? help::default_show_topic : initial)
	, current_topic_()
	, toplevel_(toplevel)
	, history_()
	, history_pos_(0)
{
	if(initial_topic_.compare(0, 2, "..") == 0) {
		initial_topic_.replace(0, 2, "+");
	} else {
		initial_topic_.insert(0, "-");
	}
}

void help_browser::pre_show()
{
	tree_view& topic_tree = find_widget<tree_view>("topic_tree");

	button& back_button = find_widget<button>("back");
	button& next_button = find_widget<button>("next");

	rich_label& topic_text = find_widget<rich_label>("topic_text");
	panel& topic_panel = find_widget<panel>("topic_panel");

	next_button.set_active(false);
	back_button.set_active(false);
	connect_signal_mouse_left_click(back_button, std::bind(&help_browser::on_history_navigate, this, true));
	connect_signal_mouse_left_click(next_button, std::bind(&help_browser::on_history_navigate, this, false));

	connect_signal<event::BACK_BUTTON_CLICK>([this](auto&&...) {
		on_history_navigate(true);
	}, event::dispatcher::front_pre_child);
	connect_signal<event::FORWARD_BUTTON_CLICK>([this](auto&&...) {
		on_history_navigate(false);
	}, event::dispatcher::front_pre_child);

	toggle_button& contents = find_widget<toggle_button>("contents");

	contents.set_value(true);
	topic_panel.set_visible(true);
	connect_signal_mouse_left_click(contents, [&](auto&&...) {
		auto parent = topic_panel.get_window();
		// Cache the initial values, get_best_size() keeps changing
		// initial width of the window
		if ((parent != nullptr) && (win_w == 0)) {
			win_w = parent->get_best_size().x;
		}

		// initial width of the rich label
		if(rl_init_w == 0) {
			rl_init_w = topic_text.get_width();
		}

		// Set RL's width and reshow
		bool is_contents_visible = (topic_panel.get_visible() == widget::visibility::visible);
		if (parent) {
			topic_text.set_width(is_contents_visible ? win_w : rl_init_w);
			show_topic(history_.at(history_pos_), false, true);
		}
		topic_panel.set_visible(!is_contents_visible);
		invalidate_layout();
	});

	text_box& filter = find_widget<text_box>("filter_box");
	add_to_keyboard_chain(&filter);
	filter.on_modified([this](const auto& box) { update_list(box.text()); });

	topic_text.register_link_callback(std::bind(&help_browser::show_topic, this, std::placeholders::_1, true, false));

	connect_signal_notify_modified(topic_tree, std::bind(&help_browser::on_topic_select, this));

	keyboard_capture(&topic_tree);

	add_topics_for_section(toplevel_, topic_tree.get_root_node());

	tree_view_node* initial_node = topic_tree.find_widget<tree_view_node>(initial_topic_, false, false);
	if(initial_node) {
		initial_node->select_node(true);
	}

	show_topic(initial_topic_);
}

void help_browser::update_list(const std::string& filter_text) {
	tree_view& topic_tree = find_widget<tree_view>("topic_tree");
	topic_tree.clear();
	if(!add_topics_for_section(toplevel_, topic_tree.get_root_node(), filter_text)) {
		// Add everything if nothing matches
		add_topics_for_section(toplevel_, topic_tree.get_root_node());
	}
}

bool help_browser::add_topics_for_section(const help::section& parent_section, tree_view_node& parent_node, const std::string& filter_text)
{
	bool topics_added = false;
	const auto match = translation::make_ci_matcher(filter_text);

	for(const help::section& section : parent_section.sections) {
		tree_view_node& section_node = add_topic(section.id, section.title, true, parent_node);
		bool subtopics_added = add_topics_for_section(section, section_node, filter_text);

		if (subtopics_added || (match(section.id) || match(section.title))) {
			if (!filter_text.empty()) {
				section_node.unfold();
			}
			topics_added = true;
		} else {
			find_widget<tree_view>("topic_tree").remove_node(&section_node);
		}
	}

	for(const help::topic& topic : parent_section.topics) {
		if (topic.id[0] == '.') {
			continue;
		}

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

void help_browser::show_topic(std::string topic_id, bool add_to_history, bool reshow)
{
	if(reshow) {
		const help::topic* topic = help::find_topic(toplevel_, topic_id);
		find_widget<rich_label>("topic_text").set_dom(topic->text.parsed_text());
		invalidate_layout();
		return;
	}

	if(topic_id.empty() || topic_id == current_topic_) {
		return;
	} else {
		current_topic_ = topic_id;
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

		std::string topic_id_temp = topic->id;
		if(topic_id_temp.compare(0, 2, "..") == 0) {
			topic_id_temp.replace(0, 2, "+");
		} else {
			topic_id_temp.insert(0, "-");
		}
		tree_view& topic_tree = find_widget<tree_view>("topic_tree");
		tree_view_node* selected_node = topic_tree.find_widget<tree_view_node>(topic_id_temp, false, false);
		if(selected_node) {
			selected_node->select_node(true, false);
		}

		find_widget<label>("topic_title").set_label(topic->title);
		try {
			find_widget<rich_label>("topic_text").set_dom(topic->text.parsed_text());
		} catch(const markup::parse_error& e) {
			find_widget<rich_label>("topic_text").set_label(
				markup::span_color(font::BAD_COLOR,
					"Error parsing markup in help page with ID: " + topic->id + "\n"
					+ font::escape_text(e.message)));
		}

		invalidate_layout();
	}

	if(add_to_history) {
		// history pos is 0 initially, so it's already at first entry
		// no need to increment first time
		if (!history_.empty()) {
			// don't add duplicate entries back-to-back
			if (history_.back() == topic_id) {
				return;
			}
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

	show_topic(selected->id());
}

void help_browser::on_history_navigate(bool backwards)
{
	if(backwards) {
		if (history_pos_ > 0) {
			history_pos_--;
		} else {
			return;
		}
	} else {
		if (history_pos_ < history_.size() - 1) {
			history_pos_++;
		} else {
			return;
		}
	}
	find_widget<button>("back").set_active(!history_.empty() && history_pos_ != 0);
	find_widget<button>("next").set_active(!history_.empty() && history_pos_ != (history_.size()-1));

	show_topic(history_.at(history_pos_), false);
}

} // namespace dialogs
