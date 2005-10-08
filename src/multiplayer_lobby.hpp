/* $Id$ */
/*
   Copyright (C)
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef MULTIPLAYER_LOBBY_HPP_INCLUDED
#define MULTIPLAYER_LOBBY_HPP_INCLUDED

#include <map>

#include "config.hpp"
#include "display.hpp"
#include "multiplayer_ui.hpp"

#include "widgets/scrollarea.hpp"

// This module controls the multiplayer lobby. A section on the server which
// allows players to chat, create games, and join games.
namespace mp {
class gamebrowser : public gui::scrollarea {
public:
	struct game_item {
		surface mini_map;
		std::string map_data;
		std::string name;
		std::string map_info;
		std::string gold;
		std::string xp;
		std::string vision;
		std::string status;
		size_t vacant_slots;
		bool fog;
		bool shroud;
		bool observers;
		bool use_map_settings;
	};
	gamebrowser(CVideo& video);
	void scroll(int pos);
	void handle_event(const SDL_Event& event);
	void set_inner_location(const SDL_Rect& rect);
	void set_item_height(unsigned int height);
	void set_game_items(const config& cfg, const config& game_config);
	void draw();
	void draw_contents() const;
	void draw_item(size_t index) const;
	SDL_Rect get_item_rect(size_t index) const;
	bool empty() const { return games_.empty(); }
	bool selection_is_joinable() const { return empty() ? false : games_[selected_].vacant_slots > 0; }
	bool selection_is_observable() const { return empty() ? false : games_[selected_].observers; }
	bool selected() const { return double_clicked_ && !empty(); }
	int selection() const { return selected_; }
protected:
private:
	image::locator gold_icon_locator_;
	image::locator xp_icon_locator_;
	image::locator vision_icon_locator_;
	image::locator observer_icon_locator_;

	unsigned int item_height_;
	int margin_;
	int h_padding_;
	int v_padding_;
	int header_height_;
	size_t selected_;
	std::pair<size_t, size_t> visible_range_;
	std::vector<game_item> games_;
	std::vector<size_t> redraw_items_;
	bool double_clicked_;
	bool ignore_next_doubleclick_;
	bool last_was_doubleclick_;
};

class lobby : public ui
{
public:
	lobby(display& d, const config& cfg, chat& c, config& gamelist);

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
	gui::button quit_game_;

	lobby_sorter sorter_;
	gamebrowser games_menu_;
	int current_game_;

	std::map<std::string,std::string> minimaps_;
};

}

#endif
