/*
   Copyright (C) 2016 - 2017 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "gui/dialogs/modal_dialog.hpp"
#include "statistics.hpp"

class CVideo;
class team;

namespace gui2
{

class menu_button;

namespace dialogs
{

class statistics_dialog : public modal_dialog
{
public:
	statistics_dialog(const team& current_team);

	DEFINE_SIMPLE_DISPLAY_WRAPPER(statistics_dialog)

private:
	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const override;

	/** Inherited from modal_dialog. */
	virtual void pre_show(window& window) override;

	/**
	 * Picks out the stats structure that was selected for displaying.
	 */
	inline const statistics::stats & current_stats();

	void add_stat_row(window& window, const std::string& type, const statistics::stats::str_int_map& value, const bool has_cost = true);

	void add_damage_row(
		window& window,
		const std::string& type,
		const long long& damage,
		const long long& expected,
		const long long& turn_damage,
		const long long& turn_expected,
		const bool show_this_turn);

	void update_lists(window& window);

	void on_primary_list_select(window& window);
	void on_scenario_select(window& window);
	void on_tab_select(window& window);

	const team& current_team_;

	const statistics::stats  campaign_;
	const statistics::levels scenarios_;

	size_t scenario_index_;

	std::vector<const statistics::stats::str_int_map*> main_stat_table_;
};

} // namespace dialogs
} // namespace gui2
