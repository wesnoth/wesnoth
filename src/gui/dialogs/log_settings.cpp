/*
	Copyright (C) 2017 - 2024
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

#include "gui/dialogs/log_settings.hpp"

#include "gettext.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/widgets/grid.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/window.hpp"

#include "log.hpp"

namespace gui2::dialogs
{

REGISTER_DIALOG(log_settings)

log_settings::log_settings()
	: modal_dialog(window_id())
	, last_words_()
{
	//list of names must match those in logging.cfg
	widget_id_.push_back("none");
	widget_id_.push_back("err");
	widget_id_.push_back("warn");
	widget_id_.push_back("info");
	widget_id_.push_back("debug");


	//empty string is the filter (in other words, this grabs the whole list of domains)
	std::string temp_string = lg::list_log_domains("");
	//std::cout<<temp_string; //use to print the full log domain list
	std::string one_domain;

	std::istringstream iss(temp_string, std::istringstream::in);

	while(iss >> one_domain){
		domain_list_.push_back(one_domain);
	}
}

void log_settings::pre_show(window& window)
{
	listbox& logger_box = find_widget<listbox>(&window, "logger_listbox", false);

	for(unsigned int i = 0; i < domain_list_.size(); i++){
		std::string this_domain = domain_list_[i];
		widget_data data;
		widget_item item;

		item["label"] = this_domain;
		data["label"] = item;

		logger_box.add_row(data);
		group<std::string>& group = groups_[this_domain];

		grid* this_grid = logger_box.get_row_grid(i);
		for(std::string this_id : widget_id_){
			widget* this_widget = this_grid->find(this_id, false);
			toggle_button* button = dynamic_cast<toggle_button*>(this_widget);
			if(button != nullptr) {
				group.add_member(button, this_id);
			}
		}
		lg::severity current_sev;
        lg::severity max_sev = lg::severity::LG_DEBUG;
		if (lg::get_log_domain_severity(this_domain, current_sev)) {
			if (current_sev <= max_sev) {
				group.set_member_states(widget_id_[static_cast<int>(current_sev) + 1]);
			}
		}
	}

	text_box* filter = find_widget<text_box>(&window, "filter_box", false, true);
	filter->set_text_changed_callback(std::bind(&log_settings::filter_text_changed, this, std::placeholders::_2));

	window.keyboard_capture(filter);
	window.add_to_keyboard_chain(&logger_box);
}

void log_settings::filter_text_changed(const std::string& text)
{
	listbox& list = find_widget<listbox>(get_window(), "logger_listbox", false);

	const std::vector<std::string> words = utils::split(text, ' ');

	if(words == last_words_) {
		return;
	}

	last_words_ = words;

	boost::dynamic_bitset<> show_items;
	show_items.resize(list.get_item_count(), true);

	if(!text.empty()) {
		for(unsigned int i = 0; i < list.get_item_count(); i++) {
			assert(i < domain_list_.size());

			bool found = false;

			for(const auto& word : words)
			{
				found = translation::ci_search(domain_list_[i], word);
				if(!found) {
					break;
				}
			}

			show_items[i] = found;
		}
	}

	list.set_row_shown(show_items);
}

void log_settings::post_show(window& /*window*/)
{
	for(std::string this_domain : domain_list_){
		set_logger(this_domain);
	}
}

void log_settings::set_logger(const std::string log_domain)
{
	std::string active_value = groups_[log_domain].get_active_member_value();

	if(active_value == widget_id_[2]){ //default value, level1: warning
		lg::set_log_domain_severity(log_domain, lg::warn());
	} else if(active_value == widget_id_[4]){ //level3: debug
		lg::set_log_domain_severity(log_domain, lg::debug());
	} else if(active_value == widget_id_[3]){ //level2: info
		lg::set_log_domain_severity(log_domain, lg::info());
	} else if(active_value == widget_id_[1]){ //level0: error
		lg::set_log_domain_severity(log_domain, lg::err());
	} else if(active_value == widget_id_[0]){ //level-1: disable
		lg::set_log_domain_severity(log_domain, lg::severity::LG_NONE);
	}
}

} // namespace dialogs
