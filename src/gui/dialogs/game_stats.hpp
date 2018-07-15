/*
   Copyright (C) 2016 - 2018 by the Battle for Wesnoth Project https://www.wesnoth.org/

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

#include <memory>
#include <string>
#include <vector>

class display;
class team;
struct team_data;

namespace gui2
{
namespace dialogs
{

class game_stats : public modal_dialog
{
public:
	game_stats(const display_context& board, const int viewing_team, int& selected_index);

	static bool execute(game_board& board, const int viewing_team, int& selected_index)
	{
		if(std::all_of(board.teams().begin(), board.teams().end(), [](team& team) { return team.hidden(); })) {
			show_transient_message("", _("No visible sides found."));
			return false;
		}

		return game_stats(board, viewing_team, selected_index).show();
	}

private:
	// TODO: don't like having this
	const display_context& board_;

	const team& viewing_team_;

	std::vector<team_data> team_data_;

	int& selected_index_;

	unit_const_ptr get_leader(const int side);

	void on_tab_select(window& window);

	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const override;

	/** Inherited from modal_dialog. */
	virtual void pre_show(window& window) override;

	/** Inherited from modal_dialog. */
	virtual void post_show(window& window) override;
};

} // namespace dialogs
} // namespace gui2
