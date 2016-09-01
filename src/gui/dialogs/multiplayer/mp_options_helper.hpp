/*
   Copyright (C) 2008 - 2016 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_MP_OPTIONS_HELPER_HPP_INCLUDED
#define GUI_DIALOGS_MP_OPTIONS_HELPER_HPP_INCLUDED

#include "game_initialization/create_engine.hpp"

class config;

namespace gui2
{

class tcontrol;
class ttoggle_button;
class ttree_view;
class twindow;

class tmp_options_helper
{
public:
	tmp_options_helper(twindow& window, ng::create_engine& create_engine);

	void update_options_list();

	const config get_options_config();

private:
	struct option_source {
		std::string level_type;
		std::string id;
		friend bool operator<(const option_source& a, const option_source& b) {
			return a.level_type < b.level_type && a.id < b.id;
		}
	};

	void display_custom_options(ttree_view& options_tree, std::string&& type, const config& data);

	template<typename T>
	void update_options_data_map(T* widget, const option_source& source);
	void update_options_data_map(ttoggle_button* widget, const option_source& source);

	void reset_options_data(const option_source& source, bool& handled, bool& halt);

	ttree_view& options_tree_;
	tcontrol& no_options_notice_;

	using option_map = std::map<std::string, config::attribute_value>;

	std::vector<option_source> visible_options_;
	std::map<option_source, option_map> options_data_;

	ng::create_engine& create_engine_;

public:
	const std::vector<option_source>& get_visible_options() const
	{
		return visible_options_;
	}

	std::map<option_source, option_map>& get_options_data()
	{
		return options_data_;
	}
};

} // namespace gui2

#endif
