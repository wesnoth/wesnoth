/*
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

#include "gui/dialogs/logging.hpp"

#include "gui/auxiliary/find_widget.hpp"
#include "gui/widgets/grid.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/window.hpp"

#include "log.hpp"

namespace gui2
{

REGISTER_DIALOG(logging)

tlogging::tlogging()
{
	//list of names must match those in logging.cfg
	widget_id_.push_back("err");
	widget_id_.push_back("warn");
	widget_id_.push_back("info");
	widget_id_.push_back("debug");

	domain_list_end_ = "the end";

	//empty string is the filter (in other words, this grabs the whole list of domains)
	std::string temp_string = lg::list_logdomains("");
	//std::cout<<temp_string; //use to print the full log domain list
	std::string one_domain;

	std::istringstream iss(temp_string, std::istringstream::in);

	while(iss >> one_domain){
		domain_list_.push_back(one_domain);
	}
	domain_list_.push_back(domain_list_end_);
}

void tlogging::pre_show(twindow& window)
{
	set_restore(true); //why is this done manually?

	tlistbox& logger_box = find_widget<tlistbox>(&window, "logger_listbox", false);

	for(unsigned int counter = 0; domain_list_[counter] != domain_list_end_; counter++){
		std::string this_domain = domain_list_[counter];
		std::map<std::string, string_map> data;
		string_map item;

		for(unsigned int iter = 3; iter < 4; iter--){
			//counting down so that order matches logging.cfg
			item["toggle_button"] = widget_id_[iter]; // id?
			data["toggle_button"] = item;
		}

		/*to prevent all the individual toggles copying the label,
		 *(which will wreck everything)
		 *this widget MUST come last... I have no clue why
		 */
		item["label"] = this_domain;
		data["label"] = item;

		logger_box.add_row(data);
		tgroup<std::string>& group = groups_[this_domain];

		tgrid* this_grid = logger_box.get_row_grid(counter);
		for(unsigned int iter = 0; iter < 4; iter++){
			twidget* this_widget = this_grid->find(widget_id_[iter], false);
			ttoggle_button* button = dynamic_cast<ttoggle_button*>(this_widget);
			if(button != nullptr) {
				group.add_member(button, widget_id_[iter]);
			}
		}
		//default value is always level 1: "warn"
		group.set_member_states(widget_id_[1]);
	}
}

void tlogging::post_show(twindow& /*window*/)
{
	for(unsigned int counter = 0; domain_list_[counter] != domain_list_end_; counter++){
		set_logger(domain_list_[counter]);
	}
}

void tlogging::set_logger(const std::string log_domain)
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

} // end namespace gui2
