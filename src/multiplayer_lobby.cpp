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
#include "gettext.hpp"

namespace mp {

lobby::lobby(display& disp, const config& cfg, chat& c, config& gamelist) :
	mp::ui(disp, cfg, c, gamelist),

	observe_game_(disp, _("Observe Game")),
	join_game_(disp, _("Join Game")),
	create_game_(disp, _("Create Game")),
	quit_game_(disp, _("Quit")),
	games_menu_(disp, std::vector<std::string>()),
	current_game_(0)
{
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

	games_menu_.set_width(xscale(832));
	games_menu_.set_location(xscale(12),yscale(42));
}

void lobby::gamelist_updated()
{
	ui::gamelist_updated();

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
				char buf[50];
				sprintf(buf,"addr %lu",(size_t)(SDL_Surface*)mini);

				image::register_image(buf,mini);

				str << "&" << buf << COLUMN_SEPARATOR;
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

		game_strings.push_back(str.str());

		game_vacant_slots_.push_back(slots != "" && slots != "0");
		game_observers_.push_back((**game)["observer"] != "no");
	}

	if(game_strings.empty()) {
		game_strings.push_back(_("<no games open>"));
	}

	games_menu_.set_items(game_strings);

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
	const bool observe = observe_game_.pressed() || games_available && !game_vacant_slots_[selection] && double_click;

	if(games_available && (observe || join_game_.pressed() || double_click)) {
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
