/*
   Copyright (C) 2003 - 2015 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef DIALOGS_H_INCLUDED
#define DIALOGS_H_INCLUDED

class attack_type;
class config;
class display;
class game_display;
class unit;
class unit_map;
class unit_type;
class terrain_type;

#include "map_location.hpp"
#include "construct_dialog.hpp"
#include "network.hpp"
#include "unit_ptr.hpp"
#include "ai/lua/unit_advancements_aspect.hpp"

namespace dialogs {


/**
 * Lets the user to select a unit advancement. This should always be used
 * from WML events, advance_unit can only be used safely for normal levels.
 */
int advance_unit_dialog(const map_location &loc);

/**
 * Actually levels a unit up. This is the other part of the low-level
 * interface to the advancing code (along with advance_unit_dialog). This needs
 * to be used to implement advances from any nonstandard situation. It does
 * not add a replay.
 */
bool animate_unit_advancement(const map_location &loc, size_t choice, const bool &fire_event = true, const bool animate = true);

void show_objectives(const std::string& scenarioname, const std::string &objectives);

void show_unit_list(display& gui);

int recruit_dialog(display& disp, std::vector<const unit_type*>& units, const std::vector<std::string>& items, int side, const std::string& title_suffix);

int recall_dialog(display& disp, const boost::shared_ptr<std::vector<unit_const_ptr > > & units, int side, const std::string& title_suffix, const int team_recall_cost);

/** Show unit-stats in a side-pane to unit-list, recall-list, etc. */
class unit_preview_pane : public gui::preview_pane
{
public:
	enum TYPE { SHOW_ALL, SHOW_BASIC };
	struct details {
		details();

#ifdef SDL_GPU
		sdl::timage image;
#else
		surface image;
#endif
	  	std::string name, type_name, race;
		int level;
		std::string alignment, traits;
		std::vector<t_string> abilities;
		int hitpoints, max_hitpoints;
		int experience, max_experience;
		std::string hp_color, xp_color;
		int movement_left, total_movement;
		std::vector<attack_type> attacks;
		std::vector<std::string> overlays;
	};

	unit_preview_pane(const gui::filter_textbox *filter = NULL,
			TYPE type = SHOW_ALL, bool left_side = true);

	bool show_above() const;
	bool left_side() const;
	void set_selection(int index);

	sdl_handler_vector handler_members();

protected:
	int index_;
	gui::button details_button_;

private:
	virtual size_t size() const = 0;
	virtual const details get_details() const = 0;
	virtual void process_event() = 0;

	void draw_contents();

	const gui::filter_textbox* filter_;
	bool weapons_;
	bool left_;
};

class units_list_preview_pane : public dialogs::unit_preview_pane
{
public:
	units_list_preview_pane(unit_const_ptr u, TYPE type = SHOW_ALL, bool left_side = true);
	units_list_preview_pane(const boost::shared_ptr<const std::vector<unit_const_ptr > > & units,
		const gui::filter_textbox *filter = NULL,
		TYPE type = SHOW_ALL, bool left_side = true);

private:
	size_t size() const;
	const details get_details() const;
	void process_event();

	boost::shared_ptr<const std::vector<unit_const_ptr > > units_;
};


class unit_types_preview_pane : public dialogs::unit_preview_pane
{
public:
	unit_types_preview_pane(
			std::vector<const unit_type*>& unit_types, const gui::filter_textbox* filterbox=NULL,
			int side = 1, TYPE type=SHOW_ALL, bool left_side=true);

private:
	size_t size() const;
	const details get_details() const;
	void process_event();

	std::vector<const unit_type*>* unit_types_;
	int side_;
};

network::connection network_send_dialog(display& disp, const std::string& msg, config& cfg, network::connection connection_num=0);
network::connection network_receive_dialog(display& disp, const std::string& msg, config& cfg, network::connection connection_num=0);
network::connection network_connect_dialog(display& disp, const std::string& msg, const std::string& hostname, int port);

} //end namespace dialogs

#endif
