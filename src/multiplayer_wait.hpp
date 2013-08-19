/*
   Copyright (C) 2007 - 2013
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef MULTIPLAYER_WAIT_HPP_INCLUDED
#define MULTIPLAYER_WAIT_HPP_INCLUDED

#include "widgets/combo.hpp"

#include "gamestatus.hpp"
#include "multiplayer_ui.hpp"
#include "show_dialog.hpp"

namespace mp {

class wait : public ui
{
public:
	wait(game_display& disp, const config& cfg, game_state& state, chat& c,
		config& gamelist, const bool first_scenario = true);
	virtual void process_event();

	void join_game(bool observe);

	void start_game();

protected:
	virtual void layout_children(const SDL_Rect& rect);
	virtual void hide_children(bool hide=true);
	virtual void process_network_data(const config& data, const network::connection sock);
private:
	class leader_preview_pane : public gui::preview_pane
	{
	public:
		leader_preview_pane(game_display& disp,
			const std::vector<const config*>& available_factions,
			const std::vector<const config*>& choosable_factions, int color,
			const bool map_settings, const bool saved_game,
			const config& side_cfg);

		bool show_above() const;
		bool left_side() const;
		void set_selection(int index);
		std::string get_selected_leader();
		std::string get_selected_gender();

		handler_vector handler_members();
	private:
		virtual void draw_contents();
		virtual void process_event();

		void init_leaders_and_genders();

		const int color_;
		gui::combo leader_combo_;
		gui::combo gender_combo_;
		size_t  selection_;

		// All factions which could be played by a side (including Random).
		const std::vector<const config*>& available_factions_;

		const std::vector<const config*>& choosable_factions_;
		std::vector<std::string> choosable_leaders_;
		std::vector<std::string> choosable_genders_;

		const config* current_faction_;
		std::string current_leader_;
		std::string current_gender_;

		const bool map_settings_;
		const bool saved_game_;

		const config& side_cfg_;
	};

	void generate_menu();

	bool download_level_data();
	bool has_level_data();

	gui::button cancel_button_;
	gui::label start_label_;
	gui::menu game_menu_;

	// int team_;

	config level_;
	game_state& state_;

	const bool first_scenario_;
	bool stop_updates_;
};

}
#endif
