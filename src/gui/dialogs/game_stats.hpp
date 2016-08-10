/*
   Copyright (C) 2016 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_GAME_STATS_HPP_INCLUDED
#define GUI_DIALOGS_GAME_STATS_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"
#include "map/location.hpp"
#include "units/ptr.hpp"

#include <memory>
#include <string>
#include <vector>

class display;
class game_board;
class team;
struct team_data;

namespace gui2
{

class ttext_;

class tgame_stats : public tdialog
{
public:
	tgame_stats(game_board& board, const int viewing_team);

	int get_selected_index() const
	{
		return selected_index_;
	}

private:
	// TODO: don't like having this
	game_board& board_;

	const team& viewing_team_;

	std::vector<team_data> team_data_;

	int selected_index_;

	unit_const_ptr get_leader(const int side);

	void on_tab_select(twindow& window);

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);
};

} // namespace gui2

#endif /* ! GUI_DIALOGS_GAME_STATS_HPP_INCLUDED */
