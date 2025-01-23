/*
	Copyright (C) 2016 - 2024
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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

class team;

namespace gui2::dialogs
{
class statistics_dialog : public modal_dialog
{
public:
	statistics_dialog(statistics_t& statistics, const team& current_team);

	DEFINE_SIMPLE_DISPLAY_WRAPPER(statistics_dialog)

private:
	virtual const std::string& window_id() const override;

	virtual void pre_show() override;

	/**
	 * Picks out the stats structure that was selected for displaying.
	 */
	inline const statistics_t::stats& current_stats();

	void add_stat_row(const std::string& type, const statistics_t::stats::str_int_map& value, const bool has_cost = true);

	/** Add a row to the Damage table */
	void add_damage_row(
		const std::string& type,
		const long long& damage,
		const long long& expected,
		const long long& turn_damage,
		const long long& turn_expected,
		const bool show_this_turn);

	/**
	 * Add a row to the Hits table
	 * @param type
	 * @param more_is_better True for "Inflicted" and false for "Taken". Affects coloring.
	 * @param by_cth
	 * @param turn_by_cth
	 * @param show_this_turn
	 */
	void add_hits_row(
		const std::string& type,
		const bool more_is_better,
		const statistics_t::stats::hitrate_map& by_cth,
		const statistics_t::stats::hitrate_map& turn_by_cth,
		const bool show_this_turn);

	void update_lists();

	void on_primary_list_select();
	void on_scenario_select();

	const team& current_team_;

	const statistics_t::stats  campaign_;
	const statistics_t::levels scenarios_;

	std::size_t selection_index_;

	std::vector<const statistics_t::stats::str_int_map*> main_stat_table_;
};

} // namespace gui2::dialogs
