/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef DIALOGS_H_INCLUDED
#define DIALOGS_H_INCLUDED

class config;
class display;

#include "map.hpp"
#include "show_dialog.hpp"
#include "unit.hpp"

#include "widgets/button.hpp"

namespace dialogs
{
//function to handle an advancing unit. If there is only one choice to advance
//to, the unit will be automatically advanced. If there is a choice, and 'random_choice'
//is true, then a unit will be selected at random. Otherwise, a dialog will be displayed
//asking the user what to advance to.
//
//note that 'loc' is not a reference, because deleting an item from the units map
//(when replacing the unit that is being advanced) will possibly invalidate the reference
void advance_unit(const game_data& info, const gamemap& map,unit_map& units, gamemap::location loc,
				  display& gui, bool random_choice=false);

bool animate_unit_advancement(const game_data& info,unit_map& units, gamemap::location loc, display& gui, size_t choice);

void show_objectives(display& disp, config& level_info);

// Ask user if I should really save the game and what name I should use
// returns 0 iff user wants to save the game
int get_save_name(display & disp, const std::string& caption, 
				const std::string& message, std::string* name,
				gui::DIALOG_TYPE dialog_type=gui::YES_NO);

//allow user to select the game they want to load. Returns the name
//of the save they want to load. Stores whether the user wants to show
//a replay of the game in show_replay. If show_replay is NULL, then
//the user will not be asked if they want to show a replay.
std::string load_game_dialog(display& disp, const config& terrain_config, const game_data& data, bool* show_replay);

void unit_speak(const config& message_info, display& disp, const unit_map& units);

/// Show a dialog where the user can navigate through files and select a
/// file. The filename is used as a starting point in the navigation and
/// contains the chosen file when the function returns.  Return the
/// index of the button pressed, or -1 if the dialog was canceled
/// through keypress.
int show_file_chooser_dialog(display &displ, std::string &filename,
                             std::string const &title, int xloc = -1, int yloc = -1);

class unit_preview_pane : public gui::preview_pane
{
public:
	enum TYPE { SHOW_ALL, SHOW_BASIC };

	unit_preview_pane(display& disp, const gamemap* map, const unit& u, TYPE type=SHOW_ALL, bool left_side=true);
	unit_preview_pane(display& disp, const gamemap* map, const std::vector<unit>& units, TYPE type=SHOW_ALL, bool left_side=true);

	bool show_above() const;
	bool left_side() const;
	void set_selection(int index);

private:
	void draw_contents();
	void process_event();

	gui::button details_button_;
	const gamemap* map_;
	const std::vector<unit>* units_;
	std::vector<unit> unit_store_;
	int index_;
	bool left_;
	bool weapons_;
};

void show_unit_description(display& disp, const unit& u);


class campaign_preview_pane : public gui::preview_pane
{
public:
	campaign_preview_pane(display& disp,std::vector<std::pair<std::string,std::string> >* descriptions);

	bool show_above() const;
	bool left_side() const;
	void set_selection(int index);

private:
	void draw_contents();

	const std::vector<std::pair<std::string,std::string> >* descriptions_;
	int index_;
};

} //end namespace dialogs

#endif
