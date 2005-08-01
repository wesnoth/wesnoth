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

#include "global.hpp"
#include "multiplayer_lobby.hpp"
#include "filesystem.hpp"
#include "font.hpp"
#include "wassert.hpp"
#include "wml_separators.hpp"
#include "game_config.hpp"
#include "gettext.hpp"
#include "show_dialog.hpp"

namespace {

std::string games_menu_heading()
{
	std::ostringstream str;
	str << HEADING_PREFIX << _("Map") << COLUMN_SEPARATOR << _("Name")
		<< COLUMN_SEPARATOR << _("Status") << COLUMN_SEPARATOR << _("Settings");
	return str.str();
}

}

namespace mp {

lobby::lobby_sorter::lobby_sorter(const config& cfg) : cfg_(cfg)
{
	set_alpha_sort(1);
}

bool lobby::lobby_sorter::column_sortable(int column) const
{
	switch(column)
	{
	case MAP_COLUMN:
	case STATUS_COLUMN:
		return true;
	default:
		return basic_sorter::column_sortable(column);
	}
}

bool lobby::lobby_sorter::less(int column, const gui::menu::item& row1, const gui::menu::item& row2) const
{
	const config* const list = cfg_.child("gamelist");
	if(list == NULL) {
		return false;
	}

	const config::child_list& games = list->get_children("game");
	if(row1.id >= games.size() || row2.id >= games.size()) {
		return false;
	}

	const config& game1 = *games[row1.id];
	const config& game2 = *games[row2.id];

	if(column == MAP_COLUMN) {
		size_t mapsize1 = game1["map_data"].size();
		if(mapsize1 == 0) {
			mapsize1 = game1["map"].size();
		}

		size_t mapsize2 = game2["map_data"].size();
		if(mapsize2 == 0) {
			mapsize2 = game2["map"].size();
		}

		return mapsize1 < mapsize2;

	} else if(column == STATUS_COLUMN) {
		const int nslots1 = atoi(game1["slots"].c_str());
		const int nslots2 = atoi(game2["slots"].c_str());

		const int turn1 = atoi(game1["turn"].c_str());
		const int turn2 = atoi(game2["turn"].c_str());

		if(nslots1 > nslots2) {
			return true;
		} else if(nslots1 < nslots2) {
			return false;
		} else {
			return turn1 < turn2;
		}
	} else {
		return basic_sorter::less(column,row1,row2);
	}

	return false;
}

lobby::lobby(display& disp, const config& cfg, chat& c, config& gamelist) :
	mp::ui(disp, _("Game Lobby"), cfg, c, gamelist),

	observe_game_(disp.video(), _("Observe Game")),
	join_game_(disp.video(), _("Join Game")),
	create_game_(disp.video(), _("Create Game")),
	quit_game_(disp.video(), _("Quit")),
	sorter_(gamelist),
	games_menu_(disp.video(), std::vector<std::string>(1,games_menu_heading()),false,-1,-1,&sorter_),
	current_game_(0)
{
	game_config::debug = false;
	const SDL_Rect pos = { 0, 0, disp.video().getx(), disp.video().gety() };
	ui::set_location(pos);
	gamelist_updated();
}

void lobby::hide_children(bool hide)
{
	ui::hide_children(hide);

	games_menu_.hide(hide);
	observe_game_.hide(hide);
	join_game_.hide(hide);
	create_game_.hide(hide);
	quit_game_.hide(hide);
}

void lobby::layout_children(const SDL_Rect& rect)
{
	ui::layout_children(rect);

	join_game_.set_location(xscale(12),yscale(7));
	observe_game_.set_location(join_game_.location().x + join_game_.location().w + 5,yscale(7));
	create_game_.set_location(observe_game_.location().x + observe_game_.location().w + 5,yscale(7));
	quit_game_.set_location(create_game_.location().x + create_game_.location().w + 5,yscale(7));

	games_menu_.set_location(client_area().x, client_area().y + title().height());
	games_menu_.set_measurements(client_area().w, client_area().h
			- title().height() - gui::ButtonVPadding);
}

void lobby::gamelist_updated(bool silent)
{
	ui::gamelist_updated(silent);

	std::vector<std::string> game_strings;
	const config* list = gamelist().child("gamelist");
	if(list == NULL) {
		// No gamelist yet. Do not update anything.
		return;
	}
	config::child_list games = list->get_children("game");
	config::child_iterator game;
	game_observers_.clear();
	game_vacant_slots_.clear();

	for(game = games.begin(); game != games.end(); ++game) {

		std::stringstream str;

		std::string map_data = (**game)["map_data"];
		if(map_data == "") {
			map_data = read_map((**game)["map"]);
		}

		if(map_data != "") {
			try {
				gamemap map(game_config(), map_data);
				const surface mini(image::getMinimap(100,100,map,0));

				//generate a unique id to show the map as
				std::stringstream id;
				id << "addr " << mini.get();
				std::string const &image_id = id.str();
				image::register_image(image_id, mini);

				str << "&" << image_id << COLUMN_SEPARATOR;
			} catch(gamemap::incorrect_format_exception& e) {
				std::cerr << "illegal map: " << e.msg_ << "\n";
			}
		} else {
			str << "(" << _("Shroud") << ")" << COLUMN_SEPARATOR;
		}

		std::string name = (**game)["name"];

		str << font::make_text_ellipsis(name, font::SIZE_NORMAL, xscale(300));

		const std::string& turn = (**game)["turn"];
		const std::string& slots = (**game)["slots"];
		int nslots = lexical_cast_default<int>(slots, 0);

		if(turn != "") {
			str << COLUMN_SEPARATOR << _("Turn") << " " << turn;
		} else if(slots != "") {
			str << COLUMN_SEPARATOR << slots << " " <<
				ngettext(_("Vacant Slot"), _("Vacant Slots"), nslots);
		}
		str << COLUMN_SEPARATOR << "  " << (**game)["mp_village_gold"] << " "
			<< _("Gold") << "  " << (**game)["experience_modifier"] << "% " << "XP";
		if((**game)["mp_use_map_settings"] == "yes")
			str << "  " << _("Use map settings");
		else if((**game)["mp_fog"] == "yes")
			str << "  " << _("Fog");

		game_strings.push_back(str.str());

		game_vacant_slots_.push_back(slots != "" && slots != "0");
		game_observers_.push_back((**game)["observer"] != "no");
	}

	if(game_strings.empty()) {
		game_strings.push_back(_("<no games open>"));
	}

	//set the items, retaining the menu positioning if possible
	games_menu_.set_items(game_strings,true,true);

	if(games_menu_.selection() >= 0 && games_menu_.selection() < int(game_vacant_slots_.size())) {
		wassert(game_vacant_slots_.size() == game_observers_.size());

		observe_game_.enable(true);
		join_game_.enable(true);
		join_game_.hide(!game_vacant_slots_[games_menu_.selection()]);
		observe_game_.hide(!game_observers_[games_menu_.selection()]);
	} else {
		observe_game_.enable(false);
		join_game_.enable(false);
	}

}

void lobby::process_event()
{
	games_menu_.process();

	int selection = games_menu_.selection();

	if(selection != current_game_ && selection >= 0 && selection < int(game_vacant_slots_.size())) {
		current_game_ = selection;
		join_game_.hide(!game_vacant_slots_[selection]);
		observe_game_.hide(!game_observers_[selection]);
	}

	const bool games_available = game_vacant_slots_.empty() == false;
	wassert(!games_available || selection >= 0 && selection < int(game_vacant_slots_.size()));
	const bool double_click = games_menu_.double_clicked();
	const bool observe = observe_game_.pressed() || games_available && !game_vacant_slots_[selection] && double_click && game_observers_[selection];

	if(games_available && (observe || ((join_game_.pressed() || double_click) && game_vacant_slots_[selection]))) {
		const size_t index = size_t(games_menu_.selection());
		const config* game = gamelist().child("gamelist");
		if (game != NULL) {
			const config::const_child_itors i = game->child_range("game");
			wassert(index < size_t(i.second - i.first));
			const std::string& id = (**(i.first+index))["id"];

			config response;
			config& join = response.add_child("join");
			join["id"] = id;
			network::send_data(response);

			if (observe) {
				set_result(OBSERVE);
			} else {
				set_result(JOIN);
			}
		}
		return;
	}

	if(create_game_.pressed()) {
		set_result(CREATE);
		return;
	}

	if(quit_game_.pressed()) {
		set_result(QUIT);
		return;
	}
}


void lobby::process_network_data(const config& data, const network::connection sock)
{
	ui::process_network_data(data, sock);
}

}
