/*
   Copyright (C) 2007 - 2015 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "dialogs.hpp"
#include "gettext.hpp"
#include "game_config_manager.hpp"
#include "game_preferences.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "game_display.hpp"
#include "log.hpp"
#include "marked-up_text.hpp"
#include "mp_game_utils.hpp"
#include "multiplayer_wait.hpp"
#include "statistics.hpp"
#include "saved_game.hpp"
#include "mp_ui_alerts.hpp"
#include "scripting/plugins/context.hpp"
#include "unit_types.hpp"
#include "wml_exception.hpp"
#include "wml_separators.hpp"
#include "formula_string_utils.hpp"

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

static lg::log_domain log_network("network");
#define DBG_NW LOG_STREAM(debug, log_network)
#define LOG_NW LOG_STREAM(info, log_network)

static lg::log_domain log_enginerefac("enginerefac");
#define LOG_RG LOG_STREAM(info, log_enginerefac)
#define ERR_RG LOG_STREAM(err, log_enginerefac)

static lg::log_domain log_mp("mp/main");
#define DBG_MP LOG_STREAM(debug, log_mp)


namespace {

const SDL_Rect leader_pane_position = {-260,-370,260,370};
const int leader_pane_border = 10;

}

namespace mp {

wait::leader_preview_pane::leader_preview_pane(game_display& disp,
	ng::flg_manager& flg, const std::string& color) :
	gui::preview_pane(disp.video()),
	flg_(flg),
	color_(color),
	combo_leader_(disp, std::vector<std::string>()),
	combo_gender_(disp, std::vector<std::string>())
{
	flg_.reset_leader_combo(combo_leader_, color_);
	flg_.reset_gender_combo(combo_gender_, color_);

	set_location(leader_pane_position);
}

void wait::leader_preview_pane::process_event()
{
	if (combo_leader_.changed() && combo_leader_.selected() >= 0) {
		flg_.set_current_leader(combo_leader_.selected());

		flg_.reset_gender_combo(combo_gender_, color_);

		set_dirty();
	}

	if (combo_gender_.changed() && combo_gender_.selected() >= 0) {
		flg_.set_current_gender(combo_gender_.selected());

		set_dirty();
	}
}

void wait::leader_preview_pane::draw_contents()
{
	bg_restore();

	surface screen = video().getSurface();

	SDL_Rect const &loc = location();
	const SDL_Rect area = sdl::create_rect(loc.x + leader_pane_border,
		loc.y + leader_pane_border,loc.w - leader_pane_border * 2,
		loc.h - leader_pane_border * 2);
	const clip_rect_setter clipper(screen, &area);

	std::string faction = flg_.current_faction()["faction"];

	const std::string recruits = flg_.current_faction()["recruit"];
	const std::vector<std::string> recruit_list = utils::split(recruits);
	std::ostringstream recruit_string;

	if (!faction.empty() && faction[0] == font::IMAGE) {
		std::string::size_type p = faction.find_first_of(COLUMN_SEPARATOR);
		if (p != std::string::npos && p < faction.size())
			faction = faction.substr(p+1);
	}

	std::string image;

	const unit_type *ut = unit_types.find(flg_.current_leader());

	if (ut) {
		const unit_type &utg = ut->get_gender_unit_type(flg_.current_gender());

		image = utg.image() + ng::get_RC_suffix(utg.flag_rgb(), color_);
	}

	for(std::vector<std::string>::const_iterator itor = recruit_list.begin();
		itor != recruit_list.end(); ++itor) {

		const unit_type *rt = unit_types.find(*itor);
		if (!rt) continue;

		if (itor != recruit_list.begin())
			recruit_string << ", ";
		recruit_string << rt->type_name();
	}

	SDL_Rect image_rect = {area.x,area.y,0,0};

	surface unit_image(image::get_image(image));

	if (!unit_image.null()) {
		image_rect.w = unit_image->w;
		image_rect.h = unit_image->h;
		sdl_blit(unit_image, NULL, screen, &image_rect);
	}

	font::draw_text(&video(), area, font::SIZE_PLUS, font::NORMAL_COLOR,
		faction, area.x + 110, area.y + 60);
	const SDL_Rect leader_rect = font::draw_text(&video(), area,
		font::SIZE_SMALL, font::NORMAL_COLOR, _("Leader: "), area.x,
		area.y + 110);
	const SDL_Rect gender_rect = font::draw_text(&video(), area,
		font::SIZE_SMALL, font::NORMAL_COLOR, _("Gender: "), area.x,
		leader_rect.y + 30 + (leader_rect.h - combo_leader_.height()) / 2);
	font::draw_wrapped_text(&video(), area, font::SIZE_SMALL,
		font::NORMAL_COLOR, _("Recruits: ") + recruit_string.str(), area.x,
		area.y + 132 + 30 + (leader_rect.h - combo_leader_.height()) / 2,
		area.w);
	combo_leader_.set_location(leader_rect.x + leader_rect.w + 16,
		leader_rect.y + (leader_rect.h - combo_leader_.height()) / 2);
	combo_gender_.set_location(leader_rect.x + leader_rect.w + 16,
		gender_rect.y + (gender_rect.h - combo_gender_.height()) / 2);
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
	if (selection >= 0) {
		flg_.set_current_faction(selection);

		flg_.reset_leader_combo(combo_leader_, color_);
		flg_.reset_gender_combo(combo_gender_, color_);

		set_dirty();
	}
}

sdl_handler_vector wait::leader_preview_pane::handler_members() {
	sdl_handler_vector h;
	h.push_back(&combo_leader_);
	h.push_back(&combo_gender_);
	return h;
}


wait::wait(game_display& disp, const config& cfg, saved_game& state,
	mp::chat& c, config& gamelist, const bool first_scenario) :
	ui(disp, _("Game Lobby"), cfg, c, gamelist),
	cancel_button_(disp.video(), first_scenario ? _("Cancel") : _("Quit")),
	start_label_(disp.video(), _("Waiting for game to start..."), font::SIZE_SMALL, font::LOBBY_COLOR),
	game_menu_(disp.video(), std::vector<std::string>(), false, -1, -1, NULL, &gui::menu::bluebg_style),
	level_(),
	state_(state),
	first_scenario_(first_scenario),
	stop_updates_(false)
{
	game_menu_.set_numeric_keypress_selection(false);
	gamelist_updated();

	plugins_context_.reset(new plugins_context("Multiplayer Wait"));

	//These structure initializers create a lobby::process_data_event
	plugins_context_->set_callback("quit", 		boost::bind(&wait::process_event_impl, this, true), 						false);
	plugins_context_->set_callback("chat",		boost::bind(&wait::send_chat_message, this, boost::bind(get_str, _1, "message"), false),	true);
}

wait::~wait()
{
	try {
		if (get_result() == QUIT) {
			state_ = saved_game();
			state_.classification().campaign_type = game_classification::CAMPAIGN_TYPE::MULTIPLAYER;
		}
	} catch (...) {}
}

void wait::process_event()
{
	process_event_impl(cancel_button_.pressed());
}

void wait::process_event_impl(bool quit)
{
	if (quit) {
		set_result(QUIT);
	}
}

void wait::join_game(bool observe)
{
	const bool download_res = download_level_data();
	if (!download_res) {
		DBG_MP << "mp wait: could not download level data, quitting...";
		set_result(QUIT);
		return;
	} else if (level_["started"].to_bool()) {
		set_result(PLAY);
		return;
	}

	if (first_scenario_) {
		state_ = saved_game();
		state_.classification().campaign_type = game_classification::CAMPAIGN_TYPE::MULTIPLAYER;

		const config* campaign = &game_config_manager::get()->
			game_config().find_child("campaign", "id",
				level_.child("multiplayer")["mp_campaign"]);

		const config* scenario = &game_config_manager::get()->
			game_config().find_child("multiplayer", "id",
				level_.child(lexical_cast<std::string>(game_classification::CAMPAIGN_TYPE::MULTIPLAYER))["id"]);

		const config* era = &game_config_manager::get()->
			game_config().find_child("era", "id", level_.child("era")["id"]);

		if (*campaign) {
			state_.classification().difficulty =
				level_.child("multiplayer")["difficulty_define"].str();
			state_.classification().campaign_define =
				(*campaign)["define"].str();
			state_.classification().campaign_xtra_defines =
				utils::split((*campaign)["extra_defines"]);
		}

		if (*scenario)
			state_.classification().scenario_define =
				(*scenario)["define"].str();

		if (*era)
			state_.classification().era_define =
				(*era)["define"].str();

		BOOST_FOREACH(const config& mod, level_.child_range("modification")) {
			const config* modification = &game_config_manager::get()->
				game_config().find_child("modification", "id", mod["id"]);
			if (*modification) {
				state_.classification().mod_defines.push_back(
					(*modification)["define"].str());
			}
		}


		// Make sure that we have the same config as host, if possible.
		game_config_manager::get()->
			load_game_config_for_game(state_.classification());
	}

	// Add the map name to the title.
	append_to_title(": " + get_scenario()["name"].t_str());
	
	game_config::add_color_info(get_scenario());
	if (!observe) {
		//search for an appropriate vacant slot. If a description is set
		//(i.e. we're loading from a saved game), then prefer to get the side
		//with the same description as our login. Otherwise just choose the first
		//available side.
		const config *side_choice = NULL;
		int side_num = -1, nb_sides = 0;
		BOOST_FOREACH(const config &sd, get_scenario().child_range("side"))
		{
			DBG_MP << "*** side " << nb_sides << "***\n" << sd.debug() << "***\n";

			if (sd["controller"] == "reserved" && sd["current_player"] == preferences::login())
			{
				side_choice = &sd;
				side_num = nb_sides;
				break;
			}
			if (sd["controller"] == "network" && sd["player_id"].empty())
			{
				if (!side_choice) { // found the first empty side
					side_choice = &sd;
					side_num = nb_sides;
				}
				if (sd["current_player"] == preferences::login()) {
					side_choice = &sd;
					side_num = nb_sides;
					break;  // found the preferred one
				}
			}
			if (sd["player_id"] == preferences::login())
			{
				//We already own a side in this game.
				generate_menu();
				return;
			}
			++nb_sides;
		}
		if (!side_choice) {
			size_t count = 0;
			for(config::child_itors its = get_scenario().child_range("side"); its.second != its.first; ++its.first) {
				++count;
			}
			DBG_MP << "could not find a side, all " << count << " sides were unsuitable\n";
			set_result(QUIT);
			return;
		}

		bool allow_changes = (*side_choice)["allow_changes"].to_bool(true);

		//if the client is allowed to choose their team, instead of having
		//it set by the server, do that here.
		if(allow_changes) {
			events::event_context context;

			const config &era = level_.child("era");
			/** @todo Check whether we have the era. If we don't inform the user. */
			if (!era)
				throw config::error(_("No era information found."));
			config::const_child_itors possible_sides = era.child_range("multiplayer_side");
			if (possible_sides.first == possible_sides.second) {
				set_result(QUIT);
				throw config::error(_("No multiplayer sides found"));
			}

			const std::string color = (*side_choice)["color"].str();

			std::vector<const config*> era_factions;
			BOOST_FOREACH(const config &side, possible_sides) {
				era_factions.push_back(&side);
			}

			const bool lock_settings =
				get_scenario()["force_lock_settings"].to_bool();
			const bool use_map_settings =
				level_.child("multiplayer")["mp_use_map_settings"].to_bool();
			const bool saved_game =
				level_.child("multiplayer")["savegame"].to_bool();

			ng::flg_manager flg(era_factions, *side_choice, lock_settings, use_map_settings,
				saved_game);

			std::vector<std::string> choices;
			BOOST_FOREACH(const config *s, flg.choosable_factions())
			{
				const config &side = *s;
				const std::string &name = side["name"];
				const std::string &icon = side["image"];

				if (!icon.empty()) {
					std::string rgb = side["flag_rgb"];
					if (rgb.empty())
						rgb = "magenta";

					choices.push_back(IMAGE_PREFIX + icon + "~RC(" + rgb + ">" +
						color + ")" + COLUMN_SEPARATOR + name);
				} else {
					choices.push_back(name);
				}
			}

			std::vector<gui::preview_pane* > preview_panes;
			leader_preview_pane leader_selector(disp(), flg, color);
			preview_panes.push_back(&leader_selector);

			const int faction_choice = gui::show_dialog(disp(), NULL,
				_("Choose your faction:"), _("Starting position: ") +
				lexical_cast<std::string>(side_num + 1), gui::OK_CANCEL,
				&choices, &preview_panes);
			if(faction_choice < 0) {
				set_result(QUIT);
				return;
			}

			config faction;
			config& change = faction.add_child("change_faction");
			change["change_faction"] = true;
			change["name"] = preferences::login();
			change["faction"] = flg.current_faction()["id"];
			change["leader"] = flg.current_leader();
			change["gender"] = flg.current_gender();
			network::send_data(faction, 0);
		}

	}

	generate_menu();
}

void wait::start_game()
{
	if (const config &stats = level_.child("statistics")) {
		statistics::fresh_stats();
		statistics::read_stats(stats);
	}
	level_to_gamestate(level_, state_);

	LOG_NW << "starting game\n";
	mp_ui_alerts::game_has_begun();
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

	if(!data["message"].empty()) {
		gui2::show_transient_message(disp().video()
				, _("Response")
				, data["message"]);
	}
	if (data["failed"].to_bool()) {
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
	} else if (const config &c = data.child("scenario_diff")) {
		LOG_NW << "received diff for scenario... applying...\n";
		/** @todo We should catch config::error and then leave the game. */
		level_.apply_diff(c);
		generate_menu();
	} else if(const config &change = data.child("change_controller")) {
		LOG_NW << "received change controller" << std::endl;
		LOG_RG << "multiplayer_wait: [change_controller]" << std::endl;
		LOG_RG << data.debug() << std::endl;
		//const int side = lexical_cast<int>(change["side"]);

		if (config & sidetochange = get_scenario().find_child("side", "side", change["side"])) {
			LOG_RG << "found side : " << sidetochange.debug() << std::endl;
			sidetochange.merge_with(change);
			LOG_RG << "changed to : " << sidetochange.debug() << std::endl;
		} else {
			LOG_RG << "change_controller didn't find any side!" << std::endl;
		}
	} else if(data.has_child("scenario") || data.has_child("snapshot") || data.child("next_scenario")) {
		level_ = first_scenario_ ? data : data.child("next_scenario");
		LOG_NW << "got some sides. Current number of sides = "
			<< get_scenario().child_count("side") << ','
			<< data.child_count("side") << '\n';
		generate_menu();
	}
}

static std::string generate_user_description(const config& side)
{
	//allow the host to overwrite it, this is needed because only the host knows the ai_algorithm.
	if(const config::attribute_value* desc = side.get("user_description")) {
		return desc->str();
	}

	std::string controller_type = side["controller"].str();
	std::string reservation = side["reserved_for"].str();
	std::string owner = side["player_id"].str();

	if(controller_type == "ai") {
		return _("Computer Player");
	}
	else if (controller_type == "null") {
		return _("(Empty slot)");
	}
	else if (controller_type == "reserved") {
		utils::string_map symbols;
		symbols["playername"] = reservation;
		return vgettext("(Reserved for $playername)",symbols);
	}
	else if(owner.empty()) {
		return _("(Vacant slot)");
	}
	else if (controller_type == "human" || controller_type == "network") {
		return owner;
	}
	else {
		ERR_RG << "Found unknown controller type:" << controller_type << std::endl;
		return _("(empty)");
	}
}

void wait::generate_menu()
{
	if (stop_updates_)
		return;

	std::vector<std::string> details;
	std::set<std::string> playerlist;

	BOOST_FOREACH(const config &sd, get_scenario().child_range("side"))
	{
		if (!sd["allow_player"].to_bool(true)) {
			continue;
		}

		std::string description = generate_user_description(sd);

		t_string side_name = sd["faction_name"];
		std::string leader_type = sd["type"];
		std::string gender_id = sd["gender"];

		// Hack: if there is a unit which can recruit, use it as a
		// leader. Necessary to display leader information when loading
		// saves.
		BOOST_FOREACH(const config &side_unit, sd.child_range("unit"))
		{
			if (side_unit["canrecruit"].to_bool()) {
				leader_type = side_unit["type"].str();
				break;
			}
		}

		if(!sd["player_id"].empty())
			playerlist.insert(sd["player_id"]);

		std::string leader_name;
		std::string leader_image;

		const unit_type *ut = unit_types.find(leader_type);

		if (ut) {
			const unit_type &utg = ut->get_gender_unit_type(gender_id);

			leader_name = utg.type_name();
#ifdef LOW_MEM
			leader_image = utg.image();
#else
			std::string RCcolor = sd["color"];

			if (RCcolor.empty())
				RCcolor = sd["side"].str();
			leader_image = utg.image() + std::string("~RC(") + utg.flag_rgb() + ">" + RCcolor + ")";
#endif
		} else {
			leader_image = ng::random_enemy_picture;
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
			} else {
				// no image prefix, just add the leader image
				// (assuming that there is also no COLUMN_SEPARATOR)
				side_name = IMAGE_PREFIX + leader_image + COLUMN_SEPARATOR + side_name.str();
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
		if (sd["allow_changes"].to_bool())
			str << sd["gold"] << ' ' << _n("multiplayer_starting_gold^Gold", "multiplayer_starting_gold^Gold", sd["gold"].to_int()) << COLUMN_SEPARATOR;

		int income_amt = sd["income"];
		if(income_amt != 0){
			str << _("(") << _("Income") << ' ';
			if(income_amt > 0)
				str << _("+");
			str << sd["income"] << _(")");
		}

		str << COLUMN_SEPARATOR << t_string::from_serialized(sd["user_team_name"].str());

		str << COLUMN_SEPARATOR << get_color_string(sd["color"].str());
		details.push_back(str.str());
	}

	game_menu_.set_items(details);

	// Uses the actual connected player list if we do not have any
	// "gamelist" user data
	if (!gamelist().child("user")) {
		set_user_list(std::vector<std::string>(playerlist.begin(), playerlist.end()), true);
	}
}

bool wait::has_level_data() const
{
	if (first_scenario_) {
		return level_.has_attribute("version") && get_scenario().has_child("side");
	} else {
		return level_.has_child("next_scenario");
	}
}

bool wait::download_level_data()
{
	DBG_MP << "download_level_data()\n";
	if (!first_scenario_) {
		// Ask for the next scenario data.
		network::send_data(config("load_next_scenario"), 0);
	}

	while (!has_level_data()) {
		network::connection data_res = dialogs::network_receive_dialog(
			disp(), _("Getting game data..."), level_);

		if (!data_res) {
			DBG_MP << "download_level_data bad results\n";
			return false;
		}
		check_response(data_res, level_);
		if (level_.child("leave_game")) {
			return false;
		}
	}

	if (!first_scenario_) {
		config cfg = level_.child("next_scenario");
		level_ = cfg;
	}

	DBG_MP << "download_level_data() success.\n";

	return true;
}

config& wait::get_scenario()
{

	if(config& scenario = level_.child("scenario"))
		return scenario;
	else if(config& snapshot = level_.child("snapshot"))
		return snapshot;
	else
		return level_;
}

const config& wait::get_scenario() const
{
	if(const config& scenario = level_.child("scenario"))
		return scenario;
	else if(const config& snapshot = level_.child("snapshot"))
		return snapshot;
	else
		return level_;
}

} // namespace mp

