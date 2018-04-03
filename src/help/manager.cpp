/*
   Copyright (C) 2018 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "help/manager.hpp"

#include "config.hpp"
#include "game_config.hpp"
#include "game_config_manager.hpp"
#include "gui/dialogs/help_browser.hpp"
#include "help/constants.hpp"
#include "help/utils.hpp"
#include "preferences/game.hpp"
#include "units/types.hpp"

namespace help
{
help_manager::help_manager()
	: help_cfg_(nullptr)
	, toplevel_section_(nullptr)
	, hidden_sections_(nullptr)
	, num_last_encountered_units_(-1)
	, num_last_encountered_terrains_(-1)
	, last_debug_state_(boost::indeterminate)
{
}

#if 0
void init_help()
{
	// Find all unit_types that have not been constructed yet and fill in the information
	// needed to create the help topics
	//unit_types.build_all(unit_type::HELP_INDEXED);

	if(preferences::encountered_units().size() != std::size_t(last_num_encountered_units) ||
	   preferences::encountered_terrains().size() != std::size_t(last_num_encountered_terrains) ||
	   last_debug_state != game_config::debug || last_num_encountered_units < 0)
	{
		// More units or terrains encountered, update the contents.
		last_num_encountered_units = preferences::encountered_units().size();
		last_num_encountered_terrains = preferences::encountered_terrains().size();
		last_debug_state = game_config::debug;

		generate_contents();
	}
}
#endif
void help_manager::open_help_browser_to(std::string show_topic)
{
	// Find all unit_types that have not been constructed yet and fill in the information
	// needed to create the help topics
	unit_types.build_all(unit_type::HELP_INDEXED);

	// Build the actual stuff to display, if we need to.
	if(content_update_needed()) {
		num_last_encountered_units_ = preferences::encountered_units().size();
		num_last_encountered_terrains_ = preferences::encountered_terrains().size();
		last_debug_state_ = game_config::debug;

		build_topic_tree();
	}

	if(show_topic.empty()) {
		show_topic = help::default_topic;
	}

	// TODO: what's this for?
	if(show_topic.compare(0, 2, "..") == 0) {
		show_topic.replace(0, 2, "+");
	} else {
		show_topic.insert(0, "-");
	}

	assert(toplevel_section_);
	gui2::dialogs::help_browser::display(*toplevel_section_, show_topic);
}

bool help_manager::content_update_needed() const
{
	return
		toplevel_section_ == nullptr ||
		preferences::encountered_units().size() != static_cast<std::size_t>(num_last_encountered_units_) ||
		preferences::encountered_terrains().size() != static_cast<std::size_t>(num_last_encountered_terrains_) ||
		last_debug_state_ != game_config::debug ||
		num_last_encountered_units_ < 0;
}

void help_manager::reset_contents()
{
	toplevel_section_.reset(nullptr);
	hidden_sections_.reset(nullptr);

	num_last_encountered_units_ = -1;
	num_last_encountered_terrains_ = -1;
}

void help_manager::update_config_pointer()
{
	help_cfg_ = &game_config_manager::get()->game_config().child_or_empty("help");
	assert(help_cfg_);
}

void help_manager::build_topic_tree()
{
	// We probaby don't need to update the pointers every time, but it's the
	// simplest way to ensure these are always valid.
	update_config_pointer();

	try {
		// Start by parsing [toplevel]. It cascades down and parses all referenced sections and topics.
		toplevel_section_ = std::make_unique<section>(help_cfg_->child_or_empty("toplevel"), this);
#if 0
		//
		// TODO: REIMPLEMENT
		//

		// Create a config object that contains everything that is not referenced from the
		// toplevel element. Read this config and save these sections and topics so that they
		// can be referenced later on when showing help about specified things, but that
		// should not be shown when opening the help browser in the default manner.
		config hidden_toplevel;
		std::ostringstream ss;

		for(const config& section : help_config.child_range("section")) {
			const std::string id = section["id"];

			if(find_section(default_toplevel, id) == nullptr) {
				// This section is not referenced from the toplevel.
				// Hence, add it to the hidden ones if it is not referenced from another section.
				if(!section_is_referenced(id, help_config)) {
					if(!ss.str().empty()) {
						ss << ",";
					}

					ss << id;
				}
			}
		}

		hidden_toplevel["sections"] = ss.str();
		ss.str("");

		for(const config &topic : help_config.child_range("topic")) {
			const std::string id = topic["id"];

			if(find_topic(default_toplevel, id) == nullptr) {
				if(!topic_is_referenced(id, help_config)) {
					if(!ss.str().empty()) {
						ss << ",";
					}

					ss << id;
				}
			}
		}

		hidden_toplevel["topics"] = ss.str();
		config hidden_cfg = help_config;

		// Change the toplevel to our new, custom built one.
		hidden_cfg.clear_children("toplevel");
		hidden_cfg.add_child("toplevel", std::move(hidden_toplevel));

		hidden_sections = parse_config(&hidden_cfg);
#endif
	} catch(const parse_error& e) {
		std::cerr << "Error while parsing help text: '" << e.message << "'" << std::endl;
	}
}

const config& help_manager::get_section_config(const std::string& id) const
{
	return help_cfg_->find_child("section", "id", id);
}

const config& help_manager::get_topic_config(const std::string& id) const
{
	return help_cfg_->find_child("topic", "id", id);
}

} // namespace help
