/*
	Copyright (C) 2003 - 2024
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
#include <list>                         // for _List_const_iterator, etc
#include <utility>                      // for pair
#include <SDL2/SDL.h>

namespace help {

help_menu::help_menu(const section& toplevel, int max_height) :
	gui::menu(true, max_height, -1, &gui::menu::bluebg_style),
	visible_items_(),
	toplevel_(toplevel),
	expanded_(),
	chosen_topic_(nullptr),
	selected_item_(&toplevel, 0)
{
	silent_ = true; //silence the default menu sounds
	update_visible_items(toplevel_);
	display_visible_items();
	if (!visible_items_.empty())
		selected_item_ = visible_items_.front();
}

bool help_menu::expanded(const section &sec) const
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
			visible_items_.emplace_back(&s, level + 1);
			if (expanded(s)) {
				update_visible_items(s, level + 1);
			}
		}
	}
	for (const auto &t : sec.topics) {
		if (is_visible_id(t.id)) {
			visible_items_.emplace_back(&t, level + 1);
		}
	}
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

	if (!visible_items_.empty()
		&& static_cast<std::size_t>(res) < visible_items_.size())
	{
		selected_item_ = visible_items_[res];
		const section* sec = selected_item_.sec;
		if (sec != nullptr) {
			// Behavior of the UI, for section headings:
			// * user single-clicks on the text part: show the ".." topic in the right-hand pane
			// * user single-clicks on the icon (or to the left of it): expand or collapse the tree view
			// * user double-clicks anywhere: expand or collapse the tree view
			// * note: the first click of the double-click has the single-click effect too
			if (menu::double_clicked() || hit_on_indent_or_icon(static_cast<std::size_t>(res), mousex)) {
				// Open or close a section if we double-click on it
				// or do simple click on the icon.
				expanded(*sec) ? contract(*sec) : expand(*sec);
				update_visible_items(toplevel_);
				display_visible_items();
			} else {
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
	std::vector<gui::indented_menu_item> menu_items;
	std::optional<std::size_t> selected;
	for(std::vector<visible_item>::const_iterator items_it = visible_items_.begin(),
		 end = visible_items_.end(); items_it != end; ++items_it) {
		if (selected_item_ == *items_it)
			selected = menu_items.size();
		menu_items.push_back(items_it->get_menu_item(*this));
	}
	set_items(menu_items, selected);
}

help_menu::visible_item::visible_item(const section *_sec, int lev) :
	t(nullptr), sec(_sec), level(lev) {}

help_menu::visible_item::visible_item(const topic *_t, int lev) :
	t(_t), sec(nullptr), level(lev) {}

gui::indented_menu_item help_menu::visible_item::get_menu_item(const help_menu& parent) const
{
	if(sec) {
		const auto& img = parent.expanded(*sec) ? open_section_img : closed_section_img;
		return {level, img, sec->title};
	}

	// As sec was a nullptr, this must have a non-null topic
	return {level, topic_img, t->title};
}

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
