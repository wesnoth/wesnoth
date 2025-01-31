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

#include "game_board.hpp"
#include "gettext.hpp"
#include "gui/dialogs/modal_dialog.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "team.hpp"

#include <string>
#include <vector>


namespace gui2::dialogs
{

class game_stats : public modal_dialog
{
public:
	game_stats(const display_context& board, const team& viewing_team, int& selected_side_number);

	static bool execute(game_board& board, const team viewing_team, int& selected_side_number)
	{
		if(std::all_of(board.teams().begin(), board.teams().end(), [](team& team) { return team.hidden(); })) {
			show_transient_message("", _("No visible sides found."));
			return false;
		}

		return game_stats(board, viewing_team, selected_side_number).show();
	}

private:
	// TODO: don't like having this
	const display_context& board_;

	const team& viewing_team_;

	std::vector<team_data> team_data_;

	int& selected_side_number_;

	unit_const_ptr get_leader(const int side);

	void on_tab_select();

	virtual const std::string& window_id() const override;

	virtual void pre_show() override;

	virtual void post_show() override;
};

} // namespace dialogs
