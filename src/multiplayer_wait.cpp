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

#include "multiplayer_wait.hpp"
#include "preferences.hpp"
#include "log.hpp"
#include "wassert.hpp"
#include "util.hpp"
#include "replay.hpp"
#include "serialization/string_utils.hpp"

#define LOG_NW lg::info(lg::network)
#define ERR_NW lg::err(lg::network)

namespace {
const SDL_Rect leader_pane_position = {-260,-370,260,370};
const int leader_pane_border = 10;
}

namespace mp {

wait::leader_preview_pane::leader_preview_pane(display& disp, const game_data* data,
		const config::child_list& side_list) :
	gui::preview_pane(disp),
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

	surface const screen = disp().video().getSurface();

	SDL_Rect const &loc = location();
	const SDL_Rect area = { loc.x + leader_pane_border, loc.y + leader_pane_border,
	                        loc.w - leader_pane_border * 2, loc.h - leader_pane_border * 2 };
	SDL_Rect clip_area = area;
	const clip_rect_setter clipper(screen,clip_area);

	if(selection_ < side_list_.size()) {
		const config& side = *side_list_[selection_];
		std::string faction = side["name"];
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

		font::draw_text(&disp(),area,font::SIZE_PLUS,font::NORMAL_COLOUR,faction,area.x + 80, area.y + 30);
		const SDL_Rect leader_rect = font::draw_text(&disp(),area,font::SIZE_SMALL,font::NORMAL_COLOUR,
				_("Leader: "),area.x, area.y + 80);
		font::draw_wrapped_text(&disp(),area,font::SIZE_SMALL,font::NORMAL_COLOUR,
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
	ui(disp, cfg, c, gamelist),

	cancel_button_(disp, _("Cancel")),
	start_label_(disp, _("Waiting for game to start...")),
	game_menu_(disp, std::vector<std::string>()),

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
			if((**s)["controller"] == "network" && (**s)["id"].empty()) {
				if((**s)["original_description"] == preferences::login()) {
					side_choice = s - sides_list.begin();
				}
			}
		}
		const bool allow_changes = (*sides_list[side_choice])["allow_changes"] != "no";

		const std::string& era = level_["era"];
		const config* const era_cfg = game_config().find_child("era","id",era);
		if(era_cfg == NULL) {
			set_result(QUIT);
			throw network::error(_("Era not available"));
			return;
		}

		const config::child_list& possible_sides = era_cfg->get_children("multiplayer_side");
		if(possible_sides.empty()) {
			set_result(QUIT);
			throw network::error(_("No multiplayer sides found"));
			return;
		}

		std::vector<std::string> choices;
		for(config::child_list::const_iterator side =
				possible_sides.begin(); side != possible_sides.end(); ++side) {
			choices.push_back((**side)["name"]);
		}

		//if the client is allowed to choose their team, instead of having
		//it set by the server, do that here.
		std::string leader_choice;
		size_t faction_choice = 0;

		if(allow_changes) {
			std::vector<gui::preview_pane* > preview_panes;
			leader_preview_pane leader_selector(disp(), &game_data_, possible_sides);
			preview_panes.push_back(&leader_selector);

			faction_choice = size_t(gui::show_dialog(disp(), NULL, "", _("Choose your side:"),
						gui::OK_ONLY, &choices, &preview_panes));
			leader_choice = leader_selector.get_selected_leader();
		}

		wassert(faction_choice < possible_sides.size());
		//team_ = faction_choice;

		config response;
		response["side"] = lexical_cast<std::string>(side_choice);
		response["name"] = preferences::login();
		response["faction"] = lexical_cast<std::string>(faction_choice);
		response["leader"] = leader_choice;

		network::send_data(response);
	}

	generate_menu();
}

const game_state& wait::get_state()
{
	return state_;
}

const config& wait::get_level()
{
	return level_;
}

void wait::start_game() 
{
	const config::child_list& sides_list = level_.get_children("side");
	for(config::child_list::const_iterator side = sides_list.begin(); 
			side != sides_list.end(); ++side) {
		if((**side)["controller"] == "network" && (**side)["id"] == preferences::login()) {
			(**side)["controller"] = preferences::client_type();
		} else if((**side)["controller"] != "null") {
			(**side)["controller"] = "network";
		}
	}

	std::cerr << level_.write() << "\n";

	// FIXME: To be reviewed
	
	//any replay data is only temporary and should be removed from
	//the level data in case we want to save the game later
	config* const replay_data = level_.child("replay");
	config replay_data_store;
	if(replay_data != NULL) {
		replay_data_store = *replay_data;
		LOG_NW << "setting replay\n";
		recorder = replay(replay_data_store);
		if(!recorder.empty()) {
			recorder.set_skip(-1);
		}

		level_.clear_children("replay");
	}

	LOG_NW << "starting game\n";
	state_.campaign_type = "multiplayer";
	state_.starting_pos = level_;
	state_.snapshot = level_;
	state_.players.clear();
	recorder.set_save_info(state_);
}

void wait::layout_children(const SDL_Rect& rect)
{
	ui::layout_children(rect);

	const SDL_Rect ca = client_area();
	int y = ca.y + ca.h - cancel_button_.height() - gui::ButtonVPadding;

	game_menu_.set_location(ca.x, ca.y);
	game_menu_.set_measurements(ca.x + ca.w, y - ca.y - gui::ButtonVPadding);
	cancel_button_.set_location(ca.x + ca.w - cancel_button_.width() - gui::ButtonHPadding, y);
	start_label_.set_location(ca.x + gui::ButtonHPadding, y + 4);
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
		LOG_NW << "received diff for scenario....applying...\n";
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

		std::string description = sd["description"];
		std::string side_name = sd["name"];
		std::string leader_type = sd["type"];

		if(!sd["id"].empty())
			playerlist.push_back(sd["id"]);

		std::string leader_name;
		std::string leader_image;
		const game_data::unit_type_map& utypes = game_data_.unit_types;
		if (utypes.find(leader_type) != utypes.end()) {
			leader_name = utypes.find(leader_type)->second.language_name();
			leader_image = utypes.find(leader_type)->second.image();
		}
		if (!leader_image.empty()) {
			// Dumps the "image" part of the faction name, if any,
			// to replace it by a picture of the actual leader
			if(side_name[0] == font::IMAGE) {
				std::string::size_type p = side_name.find_first_of(COLUMN_SEPARATOR);
				if(p != std::string::npos && p < side_name.size()) {
					side_name = IMAGE_PREFIX + leader_image + COLUMN_SEPARATOR + side_name.substr(p+1);
				}
			}
		}

		std::stringstream str;
		str << description << COLUMN_SEPARATOR << side_name << COLUMN_SEPARATOR;
		if(!leader_name.empty())
			str << "(" << leader_name << ")";
		str << COLUMN_SEPARATOR << sd["gold"] << ' ' << sgettext("unit^Gold")
			<< COLUMN_SEPARATOR << sd["team_name"];
		details.push_back(str.str());
	}

	game_menu_.set_items(details);

	// Uses the actual connected player list if we do not have any
	// "gamelist" user data
	if (gamelist().child("user") != NULL)
		set_user_list(playerlist);
}

}

