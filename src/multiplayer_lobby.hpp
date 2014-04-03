/*
   Copyright (C) 2007 - 2009
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file multiplayer_lobby.hpp */

#ifndef MULTIPLAYER_LOBBY_HPP_INCLUDED
#define MULTIPLAYER_LOBBY_HPP_INCLUDED

#include "multiplayer_ui.hpp"
#include "game_preferences.hpp"
#include "image.hpp"

class config;
class video;
class game_display;

/**
 * This module controls the multiplayer lobby.
 *
 * A section on the server which allows players to chat, create games, and join
 * games.
 */
namespace mp {
class gamebrowser : public gui::menu {
public:
	struct game_item {

		game_item() :
			mini_map(),
			id(),
			map_data(),
			name(),
			map_info(),
			map_info_size(),
			era_and_mod_info(),
			gold(),
			xp(),
			vision(),
			status(),
			time_limit(),
			vacant_slots(0),
			current_turn(0),
			reloaded(false),
			started(false),
			fog(false),
			shroud(false),
			observers(false),
			shuffle_sides(false),
			use_map_settings(false),
			verified(false),
			password_required(false),
			have_scenario(false),
			have_era(false),
			have_all_mods(false)
		{
		}

		surface mini_map;
		std::string id;
		std::string map_data;
		std::string name;
		std::string map_info;
		std::string map_info_size;
		std::string era_and_mod_info;
		std::string gold;
		std::string xp;
		std::string vision;
		std::string status;
		std::string time_limit;
		size_t vacant_slots;
		unsigned int current_turn;
		bool reloaded;
		bool started;
		bool fog;
		bool shroud;
		bool observers;
		bool shuffle_sides;
		bool use_map_settings;
		bool verified;
		bool password_required;
		bool have_scenario;
		bool have_era;
		bool have_all_mods;
	};
	gamebrowser(CVideo& video, const config &map_hashes);
	void scroll(unsigned int pos);
	void handle_event(const SDL_Event& event);
	void set_inner_location(const SDL_Rect& rect);
	void set_item_height(unsigned int height);
	void set_game_items(const config& cfg, const config& game_config);
	void draw();
	void draw_contents();
	void draw_row(const size_t row_index, const SDL_Rect& rect, ROW_TYPE type);
	SDL_Rect get_item_rect(size_t index) const;
	bool empty() const { return games_.empty(); }
	bool selection_is_joinable() const
	{ return empty() ? false : (games_[selected_].vacant_slots > 0 &&
		!games_[selected_].started &&
		games_[selected_].have_scenario &&
		games_[selected_].have_era &&
		games_[selected_].have_all_mods); }
	// Moderators may observe any game.
	bool selection_is_observable() const
	{ return empty() ? false : (games_[selected_].observers &&
		games_[selected_].have_scenario &&
		games_[selected_].have_era &&
		games_[selected_].have_all_mods) ||
		preferences::is_authenticated(); }
	bool selected() const { return double_clicked_ && !empty(); }
	void reset_selection() { double_clicked_ = false; }
	int selection() const { return selected_; }
	game_item selected_game() { return games_[selected_]; }
	void select_game(const std::string& id);
	bool game_matches_filter(const game_item& i, const config& cfg);
protected:
	unsigned int row_height() const { return item_height_ + (2 * style_->get_thickness()); }
private:
	image::locator gold_icon_locator_;
	image::locator xp_icon_locator_;
	image::locator map_size_icon_locator_;
	image::locator vision_icon_locator_;
	image::locator time_limit_icon_locator_;
	image::locator observer_icon_locator_;
	image::locator no_observer_icon_locator_;
	image::locator shuffle_sides_icon_locator_;

	const config &map_hashes_;

	unsigned int item_height_;
	int margin_;
	int minimap_size_;
	int h_padding_;
	int h_padding_image_to_text_;
	int header_height_;
	size_t selected_;
	std::pair<size_t, size_t> visible_range_;
	std::vector<game_item> games_;
	std::vector<size_t> redraw_items_;
	std::vector<int> widths_;
	bool double_clicked_;
	bool ignore_next_doubleclick_;
	bool last_was_doubleclick_;
};

class lobby : public ui
{
public:
	lobby(game_display& d, const config& cfg, chat& c, config& gamelist);

	virtual void process_event();

protected:
	virtual void hide_children(bool hide=true);
	virtual void layout_children(const SDL_Rect& rect);
	virtual void process_network_data(const config& data, const network::connection sock);

	virtual void gamelist_updated(bool silent=true);
private:

	class lobby_sorter : public gui::menu::basic_sorter
	{
		const config& cfg_;

		bool column_sortable(int column) const;
		bool less(int column, const gui::menu::item& row1, const gui::menu::item& row2) const;

		enum { MAP_COLUMN = 0, STATUS_COLUMN = 2};
	public:
		lobby_sorter(const config& cfg);
	};

	std::vector<bool> game_vacant_slots_;
	std::vector<bool> game_observers_;

	gui::button observe_game_;
	gui::button join_game_;
	gui::button create_game_;
	gui::combo replay_options_;
	gui::button game_preferences_;
	gui::button quit_game_;

	gui::button apply_filter_;
	gui::button invert_filter_;
	gui::button vacant_slots_;
	gui::button friends_in_game_;
	gui::label filter_label_;
	gui::textbox filter_text_;

	int last_selected_game_;

	lobby_sorter sorter_;
	gamebrowser games_menu_;

	std::map<std::string,std::string> minimaps_;

	std::string search_string_;
};

} // end namespace mp

#endif
