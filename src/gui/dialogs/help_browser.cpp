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
#include "gui/widgets/multi_page.hpp"
#include "gui/widgets/tree_view.hpp"
#include "gui/widgets/tree_view_node.hpp"
#include "gui/widgets/window.hpp"

#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#endif

#include "help/help.hpp"
#include "help/help_impl.hpp"
#include "font/pango/escape.hpp"
#include "font/pango/hyperlink.hpp"

namespace gui2::dialogs
{

REGISTER_DIALOG(help_browser)

help_browser::help_browser()
	: modal_dialog(window_id())
	, initial_topic_("introduction")
{
	help::init_help();
}

void help_browser::pre_show(window& window)
{
	tree_view& topic_tree = find_widget<tree_view>(&window, "topic_tree", false);

	button& back_button = find_widget<button>(&window, "back", false);
	button& next_button = find_widget<button>(&window, "next", false);

	next_button.set_visible(widget::visibility::hidden);
	back_button.set_visible(widget::visibility::hidden);
	connect_signal_mouse_left_click(back_button, std::bind(&help_browser::on_history_navigate, this, true));
	connect_signal_mouse_left_click(next_button, std::bind(&help_browser::on_history_navigate, this, false));

	connect_signal_notify_modified(topic_tree,
		std::bind(&help_browser::on_topic_select, this));

	window.keyboard_capture(&topic_tree);

	add_topics_for_section(help::default_toplevel, topic_tree.get_root_node());

	on_topic_select();
}

void help_browser::add_topics_for_section(const help::section& parent_section, tree_view_node& parent_node)
{
	for(const help::section& section : parent_section.sections) {
		tree_view_node& section_node = add_topic(section.id, section.title, true, parent_node);

		add_topics_for_section(section, section_node);
	}

	for(const help::topic& topic : parent_section.topics) {
		add_topic(topic.id, topic.title, false, parent_node);
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

static std::string format_help_text(const config& cfg)
{
	std::stringstream ss;
	for(auto item : cfg.all_children_range()) {
		if(item.key == "text") {
			ss << font::escape_text(item.cfg["text"]);
		} else if(item.key == "ref") {
			if(item.cfg["dst"].empty()) {
				std::stringstream msg;
				msg << "Ref markup must have dst attribute. Please submit a bug"
					" report if you have not modified the game files yourself. Erroneous config: " << cfg;
				throw help::parse_error(msg.str());
			};
			// TODO: Get the proper link shade from somewhere
			ss << font::format_as_link(font::escape_text(item.cfg["text"]), color_t::from_hex_string("ffff00"));
		} else if(item.key == "img") {
			if(item.cfg["src"].empty()) {
				throw help::parse_error("Img markup must have src attribute.");
			}
			// For now, just output a placeholder so we know an image is supposed to be there.
			ss << "[img]" << font::escape_text(item.cfg["src"]) << "[/img]";
		} else if(item.key == "bold") {
			if(item.cfg["text"].empty()) {
				throw help::parse_error("Bold markup must have text attribute.");
			}
			ss << "<b>" << font::escape_text(item.cfg["text"]) << "</b>";
		} else if(item.key == "italic") {
			if(item.cfg["text"].empty()) {
				throw help::parse_error("Italic markup must have text attribute.");
			}
			ss << "<i>" << font::escape_text(item.cfg["text"]) << "</i>";
		} else if(item.key == "header") {
			if(item.cfg["text"].empty()) {
				throw help::parse_error("Header markup must have text attribute.");
			}
			ss << "<big>" << font::escape_text(item.cfg["text"]) << "</big>";
		} else if(item.key == "jump") {
			// Seems like this creates something invisible?
			if(item.cfg["amount"].empty() && item.cfg["to"].empty()) {
				throw help::parse_error("Jump markup must have either a to or an amount attribute.");
			}
		} else if(item.key == "format") {
			if(item.cfg["text"].empty()) {
				throw help::parse_error("Header markup must have text attribute.");
			}

			ss << "<span";
			if(item.cfg.has_attribute("font_size")) {
				ss << " size='" << item.cfg["font_size"].to_int(font::SIZE_NORMAL) << "'";
			}

			if(item.cfg.has_attribute("color")) {
				ss << " color='" << font::string_to_color(item.cfg["color"]).to_hex_string() << "'";
			}

			if(item.cfg["bold"]) {
				ss << " font_weight='bold'";
			}

			if(item.cfg["italic"]) {
				ss << " font_style='italic'";
			}

			ss << '>' << font::escape_text(item.cfg["text"]) << "</span>";
		}
	}
	return ss.str();
}

void help_browser::on_topic_select()
{
	multi_page& topic_pages = find_widget<multi_page>(this, "topic_text_pages", false);
	tree_view& topic_tree = find_widget<tree_view>(this, "topic_tree", false);

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

	help::section& sec = help::default_toplevel;

	auto iter = parsed_pages_.find(topic_id);
	if(iter == parsed_pages_.end()) {
		const help::topic* topic = help::find_topic(sec, topic_id);
		if(topic == nullptr) {
			return;
		}

		widget_data data;
		widget_item item;

		item["label"] = format_help_text(topic->text.parsed_text());
		data.emplace("topic_text", item);

		parsed_pages_.emplace(topic_id, topic_pages.get_page_count());
		topic_pages.add_page(data);

		invalidate_layout();
	}

	if(!history_.empty()) {
		history_.erase(std::next(history_pos_), history_.end());
	}

	history_.push_back(topic_id);
	history_pos_ = std::prev(history_.end());
	find_widget<button>(this, "back", false).set_visible(widget::visibility::visible);
	find_widget<button>(this, "next", false).set_visible(widget::visibility::hidden);

	const unsigned topic_i = parsed_pages_.at(topic_id);
	topic_pages.select_page(topic_i);
}

void help_browser::on_history_navigate(bool backwards)
{
	if(backwards) {
		history_pos_--;
	} else {
		history_pos_++;
	}
	find_widget<button>(this, "back", false).set_visible(history_pos_ == history_.begin() ? widget::visibility::hidden : widget::visibility::visible);
	find_widget<button>(this, "next", false).set_visible(history_pos_ == std::prev(history_.end()) ? widget::visibility::hidden : widget::visibility::visible);
	const unsigned topic_i = parsed_pages_.at(*history_pos_);
	find_widget<multi_page>(this, "topic_text_pages", false).select_page(topic_i);
}

} // namespace dialogs
