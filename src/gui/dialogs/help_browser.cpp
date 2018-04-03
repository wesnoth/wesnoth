/*
   Copyright (C) 2017-2018 by Charles Dang <exodia339@gmail.com>
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

#include "font/pango/escape.hpp"
#include "font/pango/hyperlink.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/multi_page.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/tree_view.hpp"
#include "gui/widgets/tree_view_node.hpp"
#include "gui/widgets/window.hpp"
#include "help/section.hpp"
#include "help/topic.hpp"
#include "help/utils.hpp"

namespace gui2
{
namespace dialogs
{
REGISTER_DIALOG(help_browser)

help_browser::help_browser(const help::section& toplevel, const std::string& initial)
	: initial_topic_(initial)
	, toplevel_(toplevel)
{
}

void help_browser::pre_show(window& window)
{
	tree_view& topic_tree = find_widget<tree_view>(&window, "topic_tree", false);

	button& back_button = find_widget<button>(&window, "back", false);
	button& next_button = find_widget<button>(&window, "next", false);

	next_button.set_visible(widget::visibility::hidden);
	back_button.set_visible(widget::visibility::hidden);

	connect_signal_mouse_left_click(back_button,
		std::bind(&help_browser::on_history_navigate, this, std::ref(window), true));

	connect_signal_mouse_left_click(next_button,
		std::bind(&help_browser::on_history_navigate, this, std::ref(window), false));

	topic_tree.set_selection_change_callback(std::bind(&help_browser::on_topic_select, this, std::ref(window)));

	window.keyboard_capture(&topic_tree);

	add_topics_for_section(toplevel_, topic_tree.get_root_node());

	tree_view_node& initial_node = find_widget<tree_view_node>(&topic_tree, initial_topic_, false);
	initial_node.select_node(true);

	on_topic_select(window);
}

void help_browser::add_topics_for_section(const help::section& parent_section, tree_view_node& parent_node)
{
	for(const auto& section : parent_section.sections()) {
		tree_view_node& section_node = add_topic(section->id, section->title, true, parent_node);

		add_topics_for_section(*section, section_node);
	}

	for(const help::topic& topic : parent_section.topics()) {
		// TODO: we do actually need to generate nodes for hidden topics somewhere
		// so they can be displayed...
		if(topic.id.compare(0, 2, "..") != 0 && help::is_visible_id(topic.id)) {
			add_topic(topic.id, topic.title, false, parent_node);
		}
	}
}

tree_view_node& help_browser::add_topic(
		const std::string& topic_id, const std::string& topic_title, bool expands, tree_view_node& parent)
{
	std::map<std::string, string_map> data;
	string_map item;

	item["label"] = topic_title;
	data.emplace("topic_name", item);

	tree_view_node& new_node = parent.add_child(expands ? "section" : "topic", data);
	new_node.set_id(std::string(expands ? "+" : "-") + topic_id);

	return new_node;
}

static std::string format_help_text(const config& cfg)
{
	std::stringstream ss;
	for(auto& item : cfg.all_children_range()) {
		if(item.key == "text") {
			//ss << font::escape_text(item.cfg["text"]);
			ss << item.cfg["text"];
		} else if(item.key == "ref") {
			if(item.cfg["dst"].empty()) {
				std::stringstream msg;
				msg << "Ref markup must have dst attribute. Please submit a bug"
					   " report if you have not modified the game files yourself. Erroneous config: "
					<< cfg;
				throw help::parse_error(msg.str());
			};
			// TODO: Get the proper link shade from somewhere
			ss << font::format_as_link(font::escape_text(item.cfg["text"]), color_t::from_hex_string("ffe100"));
		} else if(item.key == "img") {
			if(item.cfg["src"].empty()) {
				throw help::parse_error("Img markup must have src attribute.");
			}
			// For now, just output a placeholder so we know an image is supposed to be there.
			ss << "[img]" << font::escape_text(item.cfg["src"]) << "[/img]";
		} else if(item.key == "jump") {
			// This appears to be something akin to tab stops.
			if(item.cfg["amount"].empty() && item.cfg["to"].empty()) {
				throw help::parse_error("Jump markup must have either a to or an amount attribute.");
			}
			ss << '\t';
		}
	}

	return ss.str();
}

void help_browser::on_topic_select(window& window)
{
	multi_page& topic_pages = find_widget<multi_page>(&window, "topic_text_pages", false);
	tree_view& topic_tree = find_widget<tree_view>(&window, "topic_tree", false);

	if(topic_tree.empty()) {
		return;
	}

	tree_view_node* selected = topic_tree.selected_item();
	assert(selected);

	std::string topic_id = selected->id();

	if(topic_id.empty()) {
		return;
	}

	if(topic_id[0] == '+') {
		topic_id.replace(topic_id.begin(), topic_id.begin() + 1, 2, '.');
	} else {
		topic_id.erase(topic_id.begin());
	}

	const help::section& sec = toplevel_;

	auto iter = parsed_pages_.find(topic_id);
	if(iter == parsed_pages_.end()) {
		const help::topic* topic = sec.find_topic(topic_id);
		if(topic == nullptr) {
			return;
		}

		std::map<std::string, string_map> data;
		string_map item;

		item["label"] = format_help_text(topic->parsed_text());
		data.emplace("topic_text", item);

		item.clear();
		item["label"] = topic->title;
		data.emplace("topic_title", item);

		parsed_pages_.emplace(topic_id, topic_pages.get_page_count());
		topic_pages.add_page(data);

		// TODO: refactor out
		window.invalidate_layout();
	}

	if(!history_.empty()) {
		history_.erase(std::next(history_pos_), history_.end());
	}

	history_.push_back(topic_id);
	history_pos_ = std::prev(history_.end());

	if(history_pos_ != history_.begin()) {
		find_widget<button>(&window, "back", false).set_visible(widget::visibility::visible);
	}

	find_widget<button>(&window, "next", false).set_visible(widget::visibility::hidden);

	const unsigned topic_i = parsed_pages_.at(topic_id);
	topic_pages.select_page(topic_i);
}

void help_browser::on_history_navigate(window& window, bool backwards)
{
	if(backwards) {
		--history_pos_;
	} else {
		++history_pos_;
	}

	find_widget<button>(&window, "back", false).set_visible(history_pos_ == history_.begin()
		? widget::visibility::hidden
		: widget::visibility::visible);

	find_widget<button>(&window, "next", false).set_visible(history_pos_ == std::prev(history_.end())
		? widget::visibility::hidden
		: widget::visibility::visible);

	const unsigned topic_i = parsed_pages_.at(*history_pos_);
	find_widget<multi_page>(&window, "topic_text_pages", false).select_page(topic_i);
}

} // namespace dialogs
} // namespace gui2
