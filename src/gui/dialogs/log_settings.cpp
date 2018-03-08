/*
   Copyright (C) 2017-2018 by the Battle for Wesnoth Project http://www.wesnoth.org/

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

#include "gui/auxiliary/find_widget.hpp"
#include "gui/widgets/grid.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/window.hpp"

#include "log.hpp"

namespace gui2
{
namespace dialogs
{

REGISTER_DIALOG(log_settings)

log_settings::log_settings()
{
	//list of names must match those in logging.cfg
	widget_id_.push_back("err");
	widget_id_.push_back("warn");
	widget_id_.push_back("info");
	widget_id_.push_back("debug");


	//empty string is the filter (in other words, this grabs the whole list of domains)
	std::string temp_string = lg::list_logdomains("");
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
		std::map<std::string, string_map> data;
		string_map item;

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
		int current_sev, max_sev = widget_id_.size() - 1;
		if (lg::get_log_domain_severity(this_domain, current_sev) && current_sev >= 0 && current_sev <= max_sev){
			group.set_member_states(widget_id_[current_sev]);
		}
	}
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

	if(active_value == widget_id_[1]){ //default value, level1: warning
		lg::set_log_domain_severity(log_domain, lg::warn());
	} else if(active_value == widget_id_[3]){ //level3: debug
		lg::set_log_domain_severity(log_domain, lg::debug());
	} else if(active_value == widget_id_[2]){ //level2: info
		lg::set_log_domain_severity(log_domain, lg::info());
	} else if(active_value == widget_id_[0]){ //level0: error
		lg::set_log_domain_severity(log_domain, lg::err());
	}
}

} // namespace dialogs
} // namespace gui2
