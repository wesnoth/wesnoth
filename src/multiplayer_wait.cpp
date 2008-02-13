/* $Id$ */
/*
   Copyright (C) 2007
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "construct_dialog.hpp"
#include "dialogs.hpp"
#include "game_display.hpp"
#include "game_events.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"
#include "leader_list.hpp"
#include "log.hpp"
#include "marked-up_text.hpp"
#include "multiplayer.hpp"
#include "multiplayer_wait.hpp"
#include "replay.hpp"
#include "statistics.hpp"
#include "util.hpp"
#include "video.hpp"
#include "wml_separators.hpp"
#include "serialization/string_utils.hpp"

#include <cassert>

#define DBG_NW LOG_STREAM(debug, network)
#define LOG_NW LOG_STREAM(info, network)
#define ERR_NW LOG_STREAM(err, network)

namespace {
const SDL_Rect leader_pane_position = {-260,-370,260,370};
const int leader_pane_border = 10;
}

namespace mp {

wait::leader_preview_pane::leader_preview_pane(game_display& disp, const game_data* data,
		const config::child_list& side_list) :
	gui::preview_pane(disp.video()),
	side_list_(side_list),
	leader_combo_(disp, std::vector<std::string>()),
	gender_combo_(disp, std::vector<std::string>()),
	leaders_(side_list, data, &leader_combo_, &gender_combo_),
	selection_(0), data_(data)
{
	set_location(leader_pane_position);
}

void wait::leader_preview_pane::process_event()
{

	if (leader_combo_.changed() || gender_combo_.changed()) {
		leaders_.set_leader_combo(&leader_combo_);
		leaders_.update_gender_list(leaders_.get_leader());
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
		std::string gender = leaders_.get_gender();

		const game_data::unit_type_map& utypes = data_->unit_types;
		std::string leader_name;
		std::string image;

		const unit_type* ut;
		const unit_type* utg;

		if (utypes.find(leader) != utypes.end() && leader != "random") {
			ut = &(utypes.find(leader)->second);
			if (!gender.empty()) {
				if (gender == "female")
					utg = &(ut->get_gender_unit_type(unit_race::FEMALE));
				else
					utg = &(ut->get_gender_unit_type(unit_race::MALE));
			} else
				utg = ut;

			leader_name = utg->language_name();
#ifdef LOW_MEM
			image = utg->image();
#else
			image = utg->image() + std::string("~RC(") + std::string(utg->flag_rgb() + ">1)");
#endif
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

		surface unit_image(image::get_image(image));

		if(!unit_image.null()) {
			image_rect.w = unit_image->w;
			image_rect.h = unit_image->h;
			SDL_BlitSurface(unit_image,NULL,screen,&image_rect);
		}

		font::draw_text(&video(),area,font::SIZE_PLUS,font::NORMAL_COLOUR,faction,area.x + 110, area.y + 60);
		const SDL_Rect leader_rect = font::draw_text(&video(),area,font::SIZE_SMALL,font::NORMAL_COLOUR,
				_("Leader: "),area.x, area.y + 110);
		const SDL_Rect gender_rect = font::draw_text(&video(),area,font::SIZE_SMALL,font::NORMAL_COLOUR,
				_("Gender: "),area.x, leader_rect.y + 30 + (leader_rect.h - leader_combo_.height()) / 2);
		font::draw_wrapped_text(&video(),area,font::SIZE_SMALL,font::NORMAL_COLOUR,
				_("Recruits: ") + recruit_string.str(),area.x, area.y + 132 + 30 + (leader_rect.h - leader_combo_.height()) / 2,
				area.w);
		leader_combo_.set_location(leader_rect.x + leader_rect.w + 16, leader_rect.y + (leader_rect.h - leader_combo_.height()) / 2);
		gender_combo_.set_location(leader_rect.x + leader_rect.w + 16, gender_rect.y + (gender_rect.h - gender_combo_.height()) / 2);
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
	leaders_.update_gender_list(leaders_.get_leader());
	set_dirty();
}

std::string wait::leader_preview_pane::get_selected_leader()
{
	return leaders_.get_leader();
}

std::string wait::leader_preview_pane::get_selected_gender()
{
	return leaders_.get_gender();
}

handler_vector wait::leader_preview_pane::handler_members() {
	handler_vector h;
	h.push_back(&leader_combo_);
	h.push_back(&gender_combo_);
	return h;
}


wait::wait(game_display& disp, const config& cfg, const game_data& data,
		mp::chat& c, config& gamelist) :
	ui(disp, _("Game Lobby"), cfg, c, gamelist),
	cancel_button_(disp.video(), _("Cancel")),
	start_label_(disp.video(), _("Waiting for game to start..."), font::SIZE_SMALL, font::LOBBY_COLOUR),
	game_menu_(disp.video(), std::vector<std::string>(), false, -1, -1, NULL, &gui::menu::bluebg_style),
	game_data_(data),
	stop_updates_(false)
{
	game_menu_.set_numeric_keypress_selection(false);
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
		network::connection data_res = dialogs::network_receive_dialog(disp(),
				_("Getting game data..."), level_);
		if (!data_res) {
			set_result(QUIT);
			return;
		}
		check_response(data_res, level_);
		if(level_.child("leave_game")) {
			set_result(QUIT);
			return;
		}
		//if we have got valid side data
		//the first condition is to make sure that we don't have another
		//WML message with a side-tag in it
		if( (level_.values.find("version") != level_.values.end()) && (level_.child("side") != NULL) )
			break;
	}

	// Add the map name to the title.
	append_to_title(": " + level_["name"]);
	
	if (!observe) {
		const config::child_list& sides_list = level_.get_children("side");

		if(sides_list.empty()) {
			set_result(QUIT);
			throw config::error(_("No multiplayer sides available in this game"));
			return;
		}

		//search for an appropriate vacant slot. If a description is set
		//(i.e. we're loading from a saved game), then prefer to get the side
		//with the same description as our login. Otherwise just choose the first
		//available side.
		int side_choice = 0;
		for(config::child_list::const_iterator s = sides_list.begin(); s != sides_list.end(); ++s) {
			if((**s)["controller"] == "network" && (**s)["description"].empty()) {
				if((**s)["save_id"] == preferences::login() || (**s)["current_player"] == preferences::login()) {
					side_choice = s - sides_list.begin();
				}
			}
		}
		const bool allow_changes = (*sides_list[side_choice])["allow_changes"] != "no";


		//if the client is allowed to choose their team, instead of having
		//it set by the server, do that here.
		std::string leader_choice, gender_choice;
		size_t faction_choice = 0;

		if(allow_changes) {
			events::event_context context;

			const config* era = level_.child("era");
			//! @todo Check whether we have the era. If we don't inform the user.
			if(era == NULL)
				throw config::error(_("No era information found."));
			const config::child_list& possible_sides =
				era->get_children("multiplayer_side");
			if(possible_sides.empty()) {
				set_result(QUIT);
				throw config::error(_("No multiplayer sides found"));
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
			gender_choice = leader_selector.get_selected_gender();

			assert(faction_choice < possible_sides.size());

			config faction;
			config& change = faction.add_child("change_faction");
			change["name"] = preferences::login();
			change["faction"] = lexical_cast<std::string>(faction_choice);
			change["leader"] = leader_choice;
			change["gender"] = gender_choice;
			network::send_data(faction, 0, true);
		}

	}

	generate_menu();
}

const game_state& wait::get_state()
{
	return state_;
}

void wait::start_game()
{
	const config* stats = level_.child("statistics");
	if(stats != NULL) {
		statistics::fresh_stats();
		statistics::read_stats(*stats);
	}



	//! @todo Instead of using level_to_gamestate reinit the state_, 
	//! this needs more testing -- Mordante
	//! It seems level_to_gamestate is needed for the start of game
	//! download, but downloads of later scenarios miss certain info
	//! and add a players section. Use players to decide between old
	//! and new way. (Of course it would be nice to unify the data
	//! stored.)
	if(level_.child("player") == 0) {
		level_to_gamestate(level_, state_, level_["savegame"] == "yes");
	} else {
	
		state_ = game_state(game_data_, level_);

		// When we observe and don't have the addon installed we still need 
		// the old way, no clue why however. Code is a copy paste of 
		// playcampaign.cpp:576 which shows an 'Unknown scenario: '$scenario|'
		// error. This seems to work and have no side effects....
		if(!state_.scenario.empty() && state_.scenario != "null") {
			DBG_NW << "Falling back to loading the old way.\n";
			level_to_gamestate(level_, state_, level_["savegame"] == "yes");
		}
	}
	// add era events after loaded
	const config* const era_cfg = level_.child("era");
	if (era_cfg != NULL && level_["savegame"] == "no") {
		game_events::add_events(era_cfg->get_children("event"),"era_events");
	}

	LOG_NW << "starting game\n";
}

game_state& wait::request_snapshot(){
	config cfg;

	cfg.add_child("snapshot_request");

	return state_;
}

void wait::layout_children(const SDL_Rect& rect)
{
	ui::layout_children(rect);

	const SDL_Rect ca = client_area();
	int y = ca.y + ca.h - cancel_button_.height();

	game_menu_.set_location(ca.x, ca.y + title().height());
	game_menu_.set_measurements(ca.w, y - ca.y - title().height()
			- gui::ButtonVPadding);
	game_menu_.set_max_width(ca.w);
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

	if(data["message"] != "") {
		/* GCC-3.3 needs a temp var otherwise compilation fails */
		gui::dialog dlg(disp(),_("Response"),data["message"],gui::OK_ONLY);
		dlg.show();
	}
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
		//! @todo We should catch config::error and then leave the game.
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

		if(sd["allow_player"] == "no") {
			continue;
		}

		std::string description = sd["user_description"];
		const std::string faction_id = sd["id"];

		t_string side_name = sd["name"];
		std::string leader_type = sd["type"];
		std::string gender_id = sd["gender"];

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
		const unit_type* ut;
		const unit_type* utg;

		if (utypes.find(leader_type) != utypes.end() && leader_type != "random") {
			ut = &(utypes.find(leader_type)->second);
			if (!gender_id.empty()) {
				if (gender_id == "female")
					utg = &(ut->get_gender_unit_type(unit_race::FEMALE));
				else
					// FIXME: this will make it look male, even if it's random. But all this
					// code will be wiped out when the MP UI gets unified, anyway.
					utg = &(ut->get_gender_unit_type(unit_race::MALE));
			} else
				utg = ut;

			leader_name = utg->language_name();
#ifdef LOW_MEM
			leader_image = utg->image();
#else
			leader_image = utg->image() + std::string("~RC(") + std::string(utg->flag_rgb() + ">" + sd["side"] + ")");
#endif
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
		str << COLUMN_SEPARATOR;
		// Don't show gold for saved games
		if(sd["allow_changes"] == "yes")
			str << sd["gold"] << ' ' << _("Gold") << COLUMN_SEPARATOR;

		int income_amt = lexical_cast_default<int>(sd["income"], 0);
		if(income_amt != 0){
			str << _("(") << _("Income") << ' ';
			if(income_amt > 0)
				str << _("+");
			str << sd["income"] << _(")");
		}

		str	<< COLUMN_SEPARATOR << sd["user_team_name"];
		int disp_color = lexical_cast_default<int>(sd["colour"], 0) - 1;
		if(!sd["colour"].empty()) {
			try {
				disp_color = game_config::color_info(sd["colour"]).index() - 1;
			} catch(config::error&) {
				//ignore
			}
		} else {
			//! @todo we fall back to the side colour, but that's ugly rather
			// make the colour mandatory in 1.5.
			disp_color = lexical_cast_default<int>(sd["side"], 0) - 1;
		}
		str << COLUMN_SEPARATOR << get_colour_string(disp_color);
		details.push_back(str.str());
	}

	game_menu_.set_items(details);

	// Uses the actual connected player list if we do not have any
	// "gamelist" user data
	if (gamelist().child("user") == NULL) {
		set_user_list(playerlist, true);
	}
}

} // namespace mp

