/*
   Copyright (C) 2008 - 2016 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_MP_CREATE_GAME_HPP_INCLUDED
#define GUI_DIALOGS_MP_CREATE_GAME_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"

#include "game_initialization/create_engine.hpp"
#include "game_initialization/configure_engine.hpp"

class config;

namespace gui2
{

class ttree_view;
class ttoggle_panel;
class twidget;

class tmp_create_game : public tdialog
{
	typedef std::pair<ng::level::TYPE, std::string> level_type_info;

public:
	tmp_create_game(const config& cfg, ng::create_engine& create_eng);

private:
	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);

	const config& cfg_;

	std::map<std::array<std::string, 2>, std::map<std::string, std::function<config::attribute_value(void)>>> visible_options_;

	ng::create_engine& create_engine_;
	std::unique_ptr<ng::configure_engine> config_engine_;

	int selected_game_index_;

	std::vector<level_type_info> level_types_;

	void update_games_list(twindow& window);
	void display_games_of_type(twindow& window, ng::level::TYPE type);

	void show_generator_settings(twindow& window);
	void regenerate_random_map(twindow& window);

	/**
	 * All fields are also in the normal field vector, but they need to be
	 * manually controlled as well so add the pointers here as well.
	 */

	tfield_bool* use_map_settings_, *fog_, *shroud_, *start_time_, *time_limit_;

	tfield_integer* turns_, *gold_, *support_, *experience_, *init_turn_limit, *turn_bonus_, *reservior_, *action_bonus_;

	template<typename widget>
	void filter_changed_callback(twindow& window, const std::string& id);

	void on_game_select(twindow& window);

	void on_tab_select(twindow& window);

	void on_mod_select(twindow& window);
	void on_era_select(twindow& window);

	void on_mod_toggle(const int index, twidget&);

	void display_custom_options(ttree_view& options_tree, std::string&& type, const std::string& id, const config& data);

	void update_options_list(twindow& window);

	void dialog_exit_hook(twindow& window);

public:
	// another map selected
	void update_details(twindow& window);

	// use_map_settings toggled (also called in other cases.)
	void update_map_settings(twindow& window);
};

} // namespace gui2

#endif
