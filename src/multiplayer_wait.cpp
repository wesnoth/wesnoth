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

#include "display.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "marked-up_text.hpp"
#include "multiplayer.hpp"
#include "multiplayer_wait.hpp"
#include "preferences.hpp"
#include "replay.hpp"
#include "statistics.hpp"
#include "util.hpp"
#include "video.hpp"
#include "wassert.hpp"
#include "wml_separators.hpp"
#include "serialization/string_utils.hpp"

#define LOG_NW LOG_STREAM(info, network)
#define ERR_NW LOG_STREAM(err, network)

namespace {
const SDL_Rect leader_pane_position = {-260,-370,260,370};
const int leader_pane_border = 10;
}

namespace mp {

wait::leader_preview_pane::leader_preview_pane(display& disp, const game_data* data,
		const config::child_list& side_list) :
	gui::preview_pane(disp.video()),
	side_list_(side_list),
	leader_combo_(disp, std::vector<std::string>()),
	leaders_(side_list, data, &leader_combo_),
	selection_(0), data_(data)
{
	set_location(leader_pane_position);
}

void wait::leader_preview_pane::process_event()
{
	if (leader_combo_.changed()) {
		set_dirty();
	}
}

void wait::leader_preview_pane::draw_contents()
{
	bg_restore();

	surface const screen = video().getSurface();

	SDL_Rect const &loc = location();
	const SDL_Rect area = { loc.x + leader_pane_border, loc.y + leader_pane_border,
	                        loc.w - leader_pane_border * 2, loc.h - leader_pane_border * 2 };
	SDL_Rect clip_area = area;
	const clip_rect_setter clipper(screen,clip_area);

	if(selection_ < side_list_.size()) {
		const config& side = *side_list_[selection_];
		std::string faction = side["faction"];

		const std::string recruits = side["recruit"];
		const std::vector<std::string> recruit_list = utils::split(recruits);
		std::ostringstream recruit_string;

		if(faction[0] == font::IMAGE) {
			std::string::size_type p = faction.find_first_of(COLUMN_SEPARATOR);
			if(p != std::string::npos && p < faction.size())
				faction = faction.substr(p+1);
		}
		std::string leader = leaders_.get_leader();

		const game_data::unit_type_map& utypes = data_->unit_types;
		std::string leader_name;
		std::string image;

		if (utypes.find(leader) != utypes.end()) {
			leader_name = utypes.find(leader)->second.language_name();
			image = utypes.find(leader)->second.image();
		}
		for(std::vector<std::string>::const_iterator itor = recruit_list.begin();
				itor != recruit_list.end(); ++itor) {

			if (utypes.find(*itor) != utypes.end()) {
				if(itor != recruit_list.begin())
					recruit_string << ", ";
				recruit_string << utypes.find(*itor)->second.language_name();
			}
		}

		SDL_Rect image_rect = {area.x,area.y,0,0};

		surface unit_image(image::get_image(image, image::UNSCALED));

		if(!unit_image.null()) {
			image_rect.w = unit_image->w;
			image_rect.h = unit_image->h;
			SDL_BlitSurface(unit_image,NULL,screen,&image_rect);
		}

		font::draw_text(&video(),area,font::SIZE_PLUS,font::NORMAL_COLOUR,faction,area.x + 80, area.y + 30);
		const SDL_Rect leader_rect = font::draw_text(&video(),area,font::SIZE_SMALL,font::NORMAL_COLOUR,
				_("Leader: "),area.x, area.y + 80);
		font::draw_wrapped_text(&video(),area,font::SIZE_SMALL,font::NORMAL_COLOUR,
				_("Recruits: ") + recruit_string.str(),area.x, area.y + 102,
				area.w);

		leader_combo_.set_location(leader_rect.x + leader_rect.w + 10, leader_rect.y + (leader_rect.h - leader_combo_.height()) / 2);
	}
}

bool wait::leader_preview_pane::show_above() const
{
	return false;
}

bool wait::leader_preview_pane::left_side() const
{
	return false;
}

void wait::leader_preview_pane::set_selection(int selection)
{
	selection_ = selection;
	leaders_.update_leader_list(selection_);
	set_dirty();
}

std::string wait::leader_preview_pane::get_selected_leader()
{
	return leaders_.get_leader();
}

wait::wait(display& disp, const config& cfg, const game_data& data, mp::chat& c, config& gamelist) :
	ui(disp, _("Game Lobby"), cfg, c, gamelist),

	cancel_button_(disp.video(), _("Cancel")),
	start_label_(disp.video(), _("Waiting for game to start..."), font::SIZE_SMALL, font::LOBBY_COLOUR),
	game_menu_(disp.video(), std::vector<std::string>()),

	game_data_(data),
	stop_updates_(false)
{
	gamelist_updated();
}

void wait::process_event()
{
	if (cancel_button_.pressed())
		set_result(QUIT);
}

void wait::join_game(bool observe)
{
	for(;;) {
		network::connection data_res = gui::network_data_dialog(disp(),
				_("Getting game data..."), level_);
		check_response(data_res, level_);

		//if we have got valid side data
		if(level_.child("gamelist") == NULL)
			break;
	}

	if (!observe) {
		const config::child_list& sides_list = level_.get_children("side");

		if(sides_list.empty()) {
			set_result(QUIT);
			throw network::error(_("No multiplayer sides available in this game"));
			return;
		}

		//search for an appropriate vacant slot. If a description is set
		//(i.e. we're loading from a saved game), then prefer to get the side
		//with the same description as our login. Otherwise just choose the first
		//available side.
		int side_choice = 0;
		for(config::child_list::const_iterator s = sides_list.begin(); s != sides_list.end(); ++s) {
			if((**s)["controller"] == "network" && (**s)["description"].empty()) {
				if((**s)["save_id"] == preferences::login()) {
					side_choice = s - sides_list.begin();
				}
			}
		}
		const bool allow_changes = (*sides_list[side_choice])["allow_changes"] != "no";


		//if the client is allowed to choose their team, instead of having
		//it set by the server, do that here.
		std::string leader_choice;
		size_t faction_choice = 0;

		if(allow_changes) {
			events::event_context context;

			const config* era = level_.child("era");
			if(era == NULL)
				throw network::error(_("Era not available"));
			const config::child_list& possible_sides =
				era->get_children("multiplayer_side");
			if(possible_sides.empty()) {
				set_result(QUIT);
				throw network::error(_("No multiplayer sides found"));
				return;
			}

			std::vector<std::string> choices;
			for(config::child_list::const_iterator side =
					possible_sides.begin(); side !=
					possible_sides.end(); ++side) {
				choices.push_back((**side)["name"]);
			}

			std::vector<gui::preview_pane* > preview_panes;
			leader_preview_pane leader_selector(disp(), &game_data_,
					possible_sides);
			preview_panes.push_back(&leader_selector);

			const int res = gui::show_dialog(disp(), NULL, "", _("Choose your side:"),
						gui::OK_CANCEL, &choices, &preview_panes);
			if(res < 0) {
				set_result(QUIT);
				return;
			}
			faction_choice = res;
			leader_choice = leader_selector.get_selected_leader();

			wassert(faction_choice < possible_sides.size());
		}

		config response;
		response["side"] = lexical_cast<std::string>(side_choice + 1);
		response["name"] = preferences::login();
		response["faction"] = lexical_cast<std::string>(faction_choice);
		response["leader"] = leader_choice;

		network::send_data(response);
	}
	else{
		//const int res = gui::show_dialog(disp(),NULL,_("Skip replay"),_("Do you want to skip the replay?"),gui::YES_NO);
		//recorder.set_skip(res == 0);
	}

	generate_menu();
}

const game_state& wait::get_state()
{
	return state_;
}

void wait::start_game()
{
	config const * const stats = level_.child("statistics");
	if(stats != NULL) {
		statistics::fresh_stats();
		statistics::read_stats(*stats);
	}

	level_to_gamestate(level_, state_);

	LOG_NW << "starting game\n";
}

void wait::layout_children(const SDL_Rect& rect)
{
	ui::layout_children(rect);

	const SDL_Rect ca = client_area();
	int y = ca.y + ca.h - cancel_button_.height();

	game_menu_.set_location(ca.x, ca.y + title().height());
	game_menu_.set_measurements(ca.w, y - ca.y - title().height()
			- gui::ButtonVPadding);
	game_menu_.set_max_height(y - ca.y - title().height() - gui::ButtonVPadding);
	cancel_button_.set_location(ca.x + ca.w - cancel_button_.width(), y);
	start_label_.set_location(ca.x, y + 4);
}

void wait::hide_children(bool hide)
{
	ui::hide_children(hide);

	cancel_button_.hide(hide);
	game_menu_.hide(hide);
}

void wait::process_network_data(const config& data, const network::connection sock)
{
	ui::process_network_data(data, sock);

	if(data["failed"] == "yes") {
		set_result(QUIT);
		return;
	} else if(data.child("stop_updates")) {
		stop_updates_ = true;
	} else if(data.child("start_game")) {
		LOG_NW << "received start_game message\n";
		set_result(PLAY);
		return;
	} else if(data.child("leave_game")) {
		set_result(QUIT);
		return;
	} else if(data.child("scenario_diff")) {
		LOG_NW << "received diff for scenario... applying...\n";
		level_.apply_diff(*data.child("scenario_diff"));
		generate_menu();
	} else if(data.child("side")) {
		level_ = data;
		LOG_NW << "got some sides. Current number of sides = "
			<< level_.get_children("side").size() << ","
			<< data.get_children("side").size() << "\n";
		generate_menu();
	}
}

void wait::generate_menu()
{
	if (stop_updates_)
		return;

	std::vector<std::string> details;
	std::vector<std::string> playerlist;

	const config::child_list& sides = level_.get_children("side");
	for(config::child_list::const_iterator s = sides.begin(); s != sides.end(); ++s) {
		const config& sd = **s;

		std::string description = sd["user_description"];
		const std::string faction_id = sd["id"];

		t_string side_name = sd["name"];
		std::string leader_type = sd["type"];

		// Hack: if there is a unit which can recruit, use it as a
		// leader. Necessary to display leader information when loading
		// saves.
		config::const_child_itors side_units = sd.child_range("unit");
		for(;side_units.first != side_units.second; ++side_units.first) {
			if((**side_units.first)["canrecruit"] == "1") {
				leader_type = (**side_units.first)["type"];
				break;
			}
		}

		if(!sd["description"].empty())
			playerlist.push_back(sd["description"]);

		std::string leader_name;
		std::string leader_image;
		const game_data::unit_type_map& utypes = game_data_.unit_types;
		if (utypes.find(leader_type) != utypes.end()) {
			leader_name = utypes.find(leader_type)->second.language_name();
			leader_image = utypes.find(leader_type)->second.image();
		} else {
			leader_image = leader_list_manager::random_enemy_picture;
		}
		if (!leader_image.empty()) {
			// Dumps the "image" part of the faction name, if any,
			// to replace it by a picture of the actual leader
			if(side_name.str()[0] == font::IMAGE) {
				std::string::size_type p =
					side_name.str().find_first_of(COLUMN_SEPARATOR);
				if(p != std::string::npos && p < side_name.size()) {
					side_name = IMAGE_PREFIX + leader_image + COLUMN_SEPARATOR + side_name.str().substr(p+1);
				}
			}
		}

		std::stringstream str;
		str << sd["side"] << ". " << COLUMN_SEPARATOR;
		str << description << COLUMN_SEPARATOR << side_name << COLUMN_SEPARATOR;
		// Mark parentheses translatable for languages like Japanese
		if(!leader_name.empty())
			str << _("(") << leader_name << _(")");
		str << COLUMN_SEPARATOR << sd["gold"] << ' ' << sgettext("unit^Gold")
			<< COLUMN_SEPARATOR << sd["team_name"];
		str << COLUMN_SEPARATOR << get_colour_string(lexical_cast_default<int>(sd["colour"], 0) - 1);
		details.push_back(str.str());
	}

	game_menu_.set_items(details);

	// Uses the actual connected player list if we do not have any
	// "gamelist" user data
	if (gamelist().child("user") == NULL) {
		set_user_list(playerlist, true);
	}
}

}

