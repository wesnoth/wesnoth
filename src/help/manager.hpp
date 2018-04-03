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

#pragma once

#include "config.hpp"
#include "help/section.hpp"
#include "help/topic.hpp"

#include <boost/logic/tribool.hpp>

#include <memory>
#include <string>

class config;

namespace help
{
class help_manager
{
public:
	help_manager();

	/**
	 * Displays the help browser.
	 *
	 * @param show_topic       The initial topic to open to.
	 *
	 * The topic tree will be regenerated as needed beforehand.
	 *
	 * @todo should we allow passing an arbitrary toplevel section here?
	 * The old code did expose a public interface to open to any topic, but it wasn't
	 * used in any user-facing code, only as an implementation helper for the other
	 * help display functions.
	 */
	void open_help_browser_to(std::string show_topic);

	/** Clears the generated section data. */
	void reset_contents();

	/***** ***** ***** ***** Data getters ***** ***** ****** *****/

	/** Returns the [section] child with the given id. */
	const config& get_section_config(const std::string& id) const;

	/** Returns the [topic] child with the given id. */
	const config& get_topic_config(const std::string& id) const;

private:
	/** Returns true if we need to regenerate the toplevel and hidden sections. */
	bool content_update_needed() const;

	void update_config_pointer();

	/** Fills in both toplevel_section_ and hidden_sections_. */
	void build_topic_tree();

	const config* help_cfg_;

	/** The default toplevel section node. */
	section::section_ptr toplevel_section_;

	/** All sections and topics not referenced from the default toplevel node. */
	section::section_ptr hidden_sections_;

	int num_last_encountered_units_;
	int num_last_encountered_terrains_;

	boost::tribool last_debug_state_;
};

} // namespace help
