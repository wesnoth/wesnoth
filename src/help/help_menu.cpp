/*
	Copyright (C) 2003 - 2022
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "help/help_menu.hpp"

#include "game_config.hpp"              // for menu_contract, menu_expand
#include "help/help_impl.hpp"                // for section, topic, topic_list, etc
#include "sound.hpp"                    // for play_UI_sound
#include "wml_separators.hpp"           // for IMG_TEXT_SEPARATOR, etc
#include "sdl/input.hpp"                // for get_mouse_state

#include <algorithm>                    // for find
#include <iostream>                     // for basic_ostream, operator<<, etc
#include <list>                         // for _List_const_iterator, etc
#include <utility>                      // for pair
#include <SDL2/SDL.h>

class CVideo;  // lines 56-56

namespace help {

help_menu::help_menu(CVideo &video, const section& toplevel, int max_height) :
	gui::menu(video, empty_string_vector, true, max_height, -1, nullptr, &gui::menu::bluebg_style),
	visible_items_(),
	toplevel_(toplevel),
	expanded_(),
	restorer_(),
	chosen_topic_(nullptr),
	selected_item_(&toplevel, "", 0)
{
	silent_ = true; //silence the default menu sounds
	update_visible_items(toplevel_);
	display_visible_items();
	if (!visible_items_.empty())
		selected_item_ = visible_items_.front();
}

bool help_menu::expanded(const section &sec)
{
	return expanded_.find(&sec) != expanded_.end();
}

void help_menu::expand(const section &sec)
{
	if (sec.id != "toplevel" && expanded_.insert(&sec).second) {
		sound::play_UI_sound(game_config::sounds::menu_expand);
	}
}

void help_menu::contract(const section &sec)
{
	if (expanded_.erase(&sec)) {
		sound::play_UI_sound(game_config::sounds::menu_contract);
	}
}

void help_menu::update_visible_items(const section &sec, unsigned level)
{
	if (level == 0) {
		// Clear if this is the top level, otherwise append items.
		visible_items_.clear();
	}
	for (const auto &s : sec.sections) {
		if (is_visible_id(s.id)) {
			const std::string vis_string = get_string_to_show(s, level + 1);
			visible_items_.emplace_back(&s, vis_string, level + 1);
			if (expanded(s)) {
				update_visible_items(s, level + 1);
			}
		}
	}
	for (const auto &t : sec.topics) {
		if (is_visible_id(t.id)) {
			const std::string vis_string = get_string_to_show(t, level + 1);
			visible_items_.emplace_back(&t, vis_string, level + 1);
		}
	}
}

std::string help_menu::indent_list(const std::string& icon, const unsigned level) {
	std::stringstream to_show;
	for (unsigned i = 1; i < level; ++i) {
		to_show << "    "; // Indent 4 spaces
	}

	to_show << IMG_TEXT_SEPARATOR << IMAGE_PREFIX << icon;
	return to_show.str();
}

std::string help_menu::get_string_to_show(const section &sec, const unsigned level)
{
	std::stringstream to_show;
	to_show << indent_list(expanded(sec) ? open_section_img : closed_section_img, level)
		 << IMG_TEXT_SEPARATOR << sec.title;
	return to_show.str();
}

std::string help_menu::get_string_to_show(const topic &topic, const unsigned level)
{
	std::stringstream to_show;
	to_show <<  indent_list(topic_img, level)
		<< IMG_TEXT_SEPARATOR << topic.title;
	return to_show.str();
}

bool help_menu::select_topic_internal(const topic &t, const section &sec)
{
	topic_list::const_iterator tit =
		std::find(sec.topics.begin(), sec.topics.end(), t);
	if (tit != sec.topics.end()) {
		// topic starting with ".." are assumed as rooted in the parent section
		// and so only expand the parent when selected
		if (t.id.size()<2 || t.id[0] != '.' || t.id[1] != '.')
			expand(sec);
		return true;
	}
	for (const auto &s : sec.sections) {
		if (select_topic_internal(t, s)) {
			expand(sec);
			return true;
		}
	}
	return false;
}

void help_menu::select_topic(const topic &t)
{
	if (selected_item_ == t) {
		// The requested topic is already selected.
		return;
	}
	if (select_topic_internal(t, toplevel_)) {
		update_visible_items(toplevel_);
		for (std::vector<visible_item>::const_iterator it = visible_items_.begin();
			 it != visible_items_.end(); ++it) {
			if (*it == t) {
				selected_item_ = *it;
				break;
			}
		}
		display_visible_items();
	}
}

int help_menu::process()
{
	int res = menu::process();
	int mousex, mousey;
	sdl::get_mouse_state(&mousex, &mousey);

	if (!visible_items_.empty() &&
            static_cast<std::size_t>(res) < visible_items_.size()) {

		selected_item_ = visible_items_[res];
		const section* sec = selected_item_.sec;
		if (sec != nullptr) {
			// Behavior of the UI, for section headings:
			// * user single-clicks on the text part: show the ".." topic in the right-hand pane
			// * user single-clicks on the icon (or to the left of it): expand or collapse the tree view
			// * user double-clicks anywhere: expand or collapse the tree view
			// * note: the first click of the double-click has the single-click effect too
			int x = mousex - menu::location().x;

			const std::string icon_img = expanded(*sec) ? open_section_img : closed_section_img;
			// the "thickness" is the width of the left border
			int text_start = style_->item_size(indent_list(icon_img, selected_item_.level)).w - style_->get_thickness();

			// NOTE: if you want to forbid click to the left of the icon
			// also check x >= text_start-image_width(icon_img)
			if (menu::double_clicked() || x < text_start) {
				// Open or close a section if we double-click on it
				// or do simple click on the icon.
				expanded(*sec) ? contract(*sec) : expand(*sec);
				update_visible_items(toplevel_);
				display_visible_items();
			} else if (x >= text_start){
				// click on title open the topic associated to this section
				chosen_topic_ = find_topic(default_toplevel, ".."+sec->id );
			}
		} else if (selected_item_.t != nullptr) {
			// Choose a topic if it is clicked.
			chosen_topic_ = selected_item_.t;
		}
	}
	return res;
}

const topic *help_menu::chosen_topic()
{
	const topic *ret = chosen_topic_;
	chosen_topic_ = nullptr;
	return ret;
}

void help_menu::display_visible_items()
{
	std::vector<std::string> menu_items;
	for(std::vector<visible_item>::const_iterator items_it = visible_items_.begin(),
		 end = visible_items_.end(); items_it != end; ++items_it) {
		std::string to_show = items_it->visible_string;
		if (selected_item_ == *items_it)
			to_show = std::string("*") + to_show;
		menu_items.push_back(to_show);
	}
	set_items(menu_items, false, true);
}

help_menu::visible_item::visible_item(const section *_sec, const std::string &vis_string, int lev) :
	t(nullptr), sec(_sec), visible_string(vis_string), level(lev) {}

help_menu::visible_item::visible_item(const topic *_t, const std::string &vis_string, int lev) :
	t(_t), sec(nullptr), visible_string(vis_string), level(lev) {}

bool help_menu::visible_item::operator==(const section &_sec) const
{
	return sec != nullptr && *sec == _sec;
}

bool help_menu::visible_item::operator==(const topic &_t) const
{
	return t != nullptr && *t == _t;
}

bool help_menu::visible_item::operator==(const visible_item &vis_item) const
{
	return t == vis_item.t && sec == vis_item.sec;
}

} // end namespace help
