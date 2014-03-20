/*
   Copyright (C) 2007 - 2014
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#include "multiplayer.hpp"

#include "dialogs.hpp"
#include "formula_string_utils.hpp"
#include "game_preferences.hpp"
#include "generators/map_create.hpp"
#include "gettext.hpp"
#include "gui/dialogs/lobby_main.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/mp_connect.hpp"
#include "gui/dialogs/mp_create_game.hpp"
#include "gui/dialogs/mp_login.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "hash.hpp"
#include "log.hpp"
#include "generators/map_create.hpp"
#include "mp_game_utils.hpp"
#include "multiplayer_configure.hpp"
#include "multiplayer_connect.hpp"
#include "multiplayer_create.hpp"
#include "multiplayer_error_codes.hpp"
#include "multiplayer_wait.hpp"
#include "multiplayer_lobby.hpp"
#include "playcampaign.hpp"
#include "playmp_controller.hpp"
#include "settings.hpp"
#include "sound.hpp"
#include "unit_id.hpp"

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

static lg::log_domain log_network("network");
#define LOG_NW LOG_STREAM(info, log_network)

static lg::log_domain log_mp("mp/main");
#define DBG_MP LOG_STREAM(debug, log_mp)

namespace {

mp::chat gamechat;
config gamelist;

class network_game_manager
{
public:
	// Add a constructor to avoid stupid warnings with some versions of GCC
	network_game_manager() {}

	~network_game_manager()
	{
		if(network::nconnections() > 0) {
			LOG_NW << "sending leave_game\n";
			config cfg;
			cfg.add_child("leave_game");
			network::send_data(cfg, 0);
			LOG_NW << "sent leave_game\n";
		}
	}
};

}

static void run_lobby_loop(display& disp, mp::ui& ui)
{
	DBG_MP << "running lobby loop" << std::endl;
	disp.video().modeChanged();
	bool first = true;
	font::cache_mode(font::CACHE_LOBBY);
	while (ui.get_result() == mp::ui::CONTINUE) {
		if (disp.video().modeChanged() || first) {
			SDL_Rect lobby_pos = create_rect(0
					, 0
					, disp.video().getx()
					, disp.video().gety());
			ui.set_location(lobby_pos);
			first = false;
		}
		// process network data first so user actions can override the result
		// or uptodate data can prevent invalid actions
		// i.e. press cancel while you receive [start_game] or press start game while someone leaves
		ui.process_network();

		events::pump();
		events::raise_process_event();
		events::raise_draw_event();

		disp.flip();
		disp.delay(20);
	}
	font::cache_mode(font::CACHE_GAME);
}

namespace {

enum server_type {
	ABORT_SERVER,
	WESNOTHD_SERVER,
	SIMPLE_SERVER
};

}

static server_type open_connection(game_display& disp, const std::string& original_host)
{
	DBG_MP << "opening connection" << std::endl;
	std::string h = original_host;

	if(h.empty()) {
		gui2::tmp_connect dlg;

		dlg.show(disp.video());
		if(dlg.get_retval() == gui2::twindow::OK) {
			h = preferences::network_host();
		} else {
			return ABORT_SERVER;
		}
	}

	network::connection sock;

	const int pos = h.find_first_of(":");
	std::string host;
	unsigned int port;

	if(pos == -1) {
		host = h;
		port = 15000;
	} else {
		host = h.substr(0, pos);
		port = lexical_cast_default<unsigned int>(h.substr(pos + 1), 15000);
	}

	// shown_hosts is used to prevent the client being locked in a redirect
	// loop.
	typedef std::pair<std::string, int> hostpair;
	std::set<hostpair> shown_hosts;
	shown_hosts.insert(hostpair(host, port));

	config data;
	sock = dialogs::network_connect_dialog(disp,_("Connecting to Server..."),host,port);

	do {

		if (!sock) {
			return ABORT_SERVER;
		}

		data.clear();
		network::connection data_res = dialogs::network_receive_dialog(
				disp,_("Reading from Server..."),data);
		if (!data_res) return ABORT_SERVER;
		mp::check_response(data_res, data);

		if (data.has_child("reject") || data.has_attribute("version")) {
			std::string version;
			if (const config &reject = data.child("reject")) {
				version = reject["accepted_versions"].str();
			} else {
				// Backwards-compatibility "version" attribute
				version = data["version"].str();
			}

			utils::string_map i18n_symbols;
			i18n_symbols["version1"] = version;
			i18n_symbols["version2"] = game_config::version;
			const std::string errorstring = vgettext("The server accepts versions '$version1' while you are using version '$version2'", i18n_symbols);
			throw network::error(errorstring);
		}

		// Check for "redirect" messages
		if (const config &redirect = data.child("redirect"))
		{
			host = redirect["host"].str();
			port =redirect["port"].to_int(15000);

			if(shown_hosts.find(hostpair(host,port)) != shown_hosts.end()) {
				throw network::error(_("Server-side redirect loop"));
			}
			shown_hosts.insert(hostpair(host, port));

			if(network::nconnections() > 0)
				network::disconnect();
			sock = dialogs::network_connect_dialog(disp,_("Connecting to Server..."),host,port);
			continue;
		}

		if(data.child("version")) {
			config cfg;
			config res;
			cfg["version"] = game_config::version;
			res.add_child("version", cfg);
			network::send_data(res, 0);
		}

		//if we got a direction to login
		if(data.child("mustlogin")) {

			for(;;) {
				std::string password_reminder = "";

				std::string login = preferences::login();

				config response ;
				config &sp = response.add_child("login") ;
				sp["username"] = login ;

				// Login and enable selective pings -- saves server bandwidth
				// If ping_timeout has a non-zero value, do not enable
				// selective pings as this will cause clients to falsely
				// believe the server has died and disconnect.
				if (preferences::get_ping_timeout()) {
					// Pings required so disable selective pings
					sp["selective_ping"] = false;
				} else {
					// Client is bandwidth friendly so allow
					// server to optimize ping frequency as needed.
					sp["selective_ping"] = true;
				}
				network::send_data(response, 0);

				// Get response for our login request...
				network::connection data_res = network::receive_data(data, 0, 3000);
				if(!data_res) {
					throw network::error(_("Connection timed out"));
				}

				config *warning = &data.child("warning");

				if(*warning) {
					std::string warning_msg;

					utils::string_map i18n_symbols;
					i18n_symbols["nick"] = login;

					if((*warning)["warning_code"] == MP_NAME_INACTIVE_WARNING) {
						warning_msg = vgettext("The nickname ‘$nick’ is inactive. "
							"You cannot claim ownership of this nickname until you "
							"activate your account via email or ask an "
							"administrator to do it for you.", i18n_symbols);
					} else {
						warning_msg = (*warning)["message"].str();
					}

					warning_msg += "\n\n";
					warning_msg += _("Do you want to continue?");

					if(gui2::show_message(disp.video(), _("Warning"), warning_msg, gui2::tmessage::yes_no_buttons) != gui2::twindow::OK) {
						return ABORT_SERVER;
					}
				}

				config *error = &data.child("error");

				// ... and get us out of here if the server did not complain
				if (!*error) break;

				do {
					std::string password = preferences::password();

					bool fall_through = (*error)["force_confirmation"].to_bool() ?
						(gui2::show_message(disp.video(), _("Confirm"), (*error)["message"], gui2::tmessage::ok_cancel_buttons) == gui2::twindow::CANCEL) :
						false;

					const bool is_pw_request = !((*error)["password_request"].empty()) && !(password.empty());

					// If the server asks for a password, provide one if we can
					// or request a password reminder.
					// Otherwise or if the user pressed 'cancel' in the confirmation dialog
					// above go directly to the username/password dialog
					if((is_pw_request || !password_reminder.empty()) && !fall_through) {
						if(is_pw_request) {
							if ((*error)["phpbb_encryption"].to_bool())
							{

								// Apparently HTML key-characters are passed to the hashing functions of phpbb in this escaped form.
								// I will do closer investigations on this, for now let's just hope these are all of them.

								// Note: we must obviously replace '&' first, I wasted some time before I figured that out... :)
								for(std::string::size_type pos = 0; (pos = password.find('&', pos)) != std::string::npos; ++pos )
									password.replace(pos, 1, "&amp;");
								for(std::string::size_type pos = 0; (pos = password.find('\"', pos)) != std::string::npos; ++pos )
									password.replace(pos, 1, "&quot;");
								for(std::string::size_type pos = 0; (pos = password.find('<', pos)) != std::string::npos; ++pos )
									password.replace(pos, 1, "&lt;");
								for(std::string::size_type pos = 0; (pos = password.find('>', pos)) != std::string::npos; ++pos )
									password.replace(pos, 1, "&gt;");

								const std::string salt = (*error)["salt"];

								if (salt.length() < 12) {
									throw network::error(_("Bad data received from server"));
								}

								sp["password"] = util::create_hash(util::create_hash(password, util::get_salt(salt),
										util::get_iteration_count(salt)), salt.substr(12, 8));

							} else {
								sp["password"] = password;
							}
						}

						sp["password_reminder"] = password_reminder;

						// Once again send our request...
						network::send_data(response, 0);

						network::connection data_res = network::receive_data(data, 0, 3000);
						if(!data_res) {
							throw network::error(_("Connection timed out"));
						}

						error = &data.child("error");

						// ... and get us out of here if the server is happy now
						if (!*error) break;


					}

					password_reminder = "";

					// Providing a password either was not attempted because we did not
					// have any or failed:
					// Now show a dialog that displays the error and allows to
					// enter a new user name and/or password

					std::string error_message;
					utils::string_map i18n_symbols;
					i18n_symbols["nick"] = login;

					if((*error)["error_code"] == MP_MUST_LOGIN) {
						error_message = _("You must login first.");
					} else if((*error)["error_code"] == MP_NAME_TAKEN_ERROR) {
						error_message = vgettext("The nickname ‘$nick’ is already taken.", i18n_symbols);
					} else if((*error)["error_code"] == MP_INVALID_CHARS_IN_NAME_ERROR) {
						error_message = vgettext("The nickname ‘$nick’ contains invalid "
								"characters. Only alpha-numeric characters, underscores and "
								"hyphens are allowed.", i18n_symbols);
					} else if((*error)["error_code"] == MP_NAME_TOO_LONG_ERROR) {
						error_message = vgettext("The nickname ‘$nick’ is too long. Nicks must "
								"be 20 characters or less.", i18n_symbols);
					} else if((*error)["error_code"] == MP_NAME_RESERVED_ERROR) {
						error_message = vgettext("The nickname ‘$nick’ is reserved and cannot be used by players.", i18n_symbols);
					} else if((*error)["error_code"] == MP_NAME_UNREGISTERED_ERROR) {
						error_message = vgettext("The nickname ‘$nick’ is not registered on this server.", i18n_symbols)
								+ _(" This server disallows unregistered nicknames.");
					} else if((*error)["error_code"] == MP_PASSWORD_REQUEST) {
						error_message = vgettext("The nickname ‘$nick’ is registered on this server.", i18n_symbols);
					} else if((*error)["error_code"] == MP_PASSWORD_REQUEST_FOR_LOGGED_IN_NAME) {
						error_message = vgettext("The nickname ‘$nick’ is registered on this server.", i18n_symbols)
								+ "\n\n" + _("WARNING: There is already a client using this nickname, "
								"logging in will cause that client to be kicked!");
					} else if((*error)["error_code"] == MP_NO_SEED_ERROR) {
						error_message = _("Error in the login procedure (the server had no "
								"seed for your connection).");
					} else if((*error)["error_code"] == MP_INCORRECT_PASSWORD_ERROR) {
						error_message = _("The password you provided was incorrect.");
					} else if((*error)["error_code"] == MP_TOO_MANY_ATTEMPTS_ERROR) {
						error_message = _("You have made too many login attempts.");
					} else {
						error_message = (*error)["message"].str();
					}

					gui2::tmp_login dlg(error_message, !((*error)["password_request"].empty()));
					dlg.show(disp.video());

					switch(dlg.get_retval()) {
						//Log in with password
						case gui2::twindow::OK:
							break;
						//Request a password reminder
						case 1:
							password_reminder = "yes";
							break;
						// Cancel
						default: return ABORT_SERVER;
					}

				// If we have got a new username we have to start all over again
				} while(login == preferences::login());

				// Somewhat hacky...
				// If we broke out of the do-while loop above error
				// is still going to be NULL
				if(!*error) break;
			} // end login loop
		}
	} while(!(data.child("join_lobby") || data.child("join_game")));

	if (h != preferences::server_list().front().address)
		preferences::set_network_host(h);

	if (data.child("join_lobby")) {
		return WESNOTHD_SERVER;
	} else {
		return SIMPLE_SERVER;
	}

}


// The multiplayer logic consists in 4 screens:
//
// lobby <-> create <-> connect <-> (game)
//       <------------> wait    <-> (game)
//
// To each of this screen corresponds a dialog, each dialog being defined in
// the multiplayer_(screen) file.
//
// The functions enter_(screen)_mode are simple functions that take care of
// creating the dialogs, then, according to the dialog result, of calling other
// of those screen functions.

static void enter_wait_mode(game_display& disp, const config& game_config,
	game_state& state, bool observe)
{
	DBG_MP << "entering wait mode" << std::endl;

	mp::ui::result res;
	network_game_manager m;

	gamelist.clear();
	statistics::fresh_stats();
	n_unit::id_manager::instance().clear(); /* reset the unit underlying_id counter back to zero */

	{
		mp::wait ui(disp, game_config, state, gamechat, gamelist);

		ui.join_game(observe);

		run_lobby_loop(disp, ui);
		res = ui.get_result();

		if (res == mp::ui::PLAY) {
			ui.start_game();
			// FIXME commented a pointeless if since the else does exactly the same thing
			//if (preferences::skip_mp_replay()){
				//FIXME implement true skip replay
				//state = ui.request_snapshot();
			//}
		}
	}

	switch (res) {
	case mp::ui::PLAY:
		play_game(disp, state, game_config, IO_CLIENT,
			preferences::skip_mp_replay() && observe, true, preferences::blindfold_replay() && observe);
		recorder.clear();

		break;
	case mp::ui::QUIT:
	default:
		break;
	}
}

static void enter_create_mode(game_display& disp, const config& game_config,
	game_state& state, bool local_players_only = false);

static bool enter_connect_mode(game_display& disp, const config& game_config,
	game_state& state, const mp_game_settings& params,
	bool local_players_only = false)
{
	DBG_MP << "entering connect mode" << std::endl;

	mp::ui::result res;
	const network::manager net_manager(1,1);
	network_game_manager m;

	gamelist.clear();
	statistics::fresh_stats();

	{
		mp::connect_engine_ptr connect_engine(new mp::connect_engine(disp,
			state, params, local_players_only, true));
		mp::connect ui(disp, params.name, game_config, gamechat, gamelist,
			*connect_engine);
		run_lobby_loop(disp, ui);

		res = ui.get_result();

		// start_game() updates the parameters to reflect game start,
		// so it must be called before get_level()
		if (res == mp::ui::PLAY) {
			ui.start_game();
		}
	}

	switch (res) {
	case mp::ui::PLAY:
		play_game(disp, state, game_config, IO_SERVER, false,
			!local_players_only);
		recorder.clear();

		break;
	case mp::ui::CREATE:
		enter_create_mode(disp, game_config, state, local_players_only);
		break;
	case mp::ui::QUIT:
	default:
		network::send_data(config("refresh_lobby"), 0);
		return false;
	}

	return true;
}

static bool enter_configure_mode(game_display& disp, const config& game_config,
	game_state& state, const mp_game_settings& params,
	bool local_players_only = false);

static void enter_create_mode(game_display& disp, const config& game_config,
	game_state& state, bool local_players_only)
{
	DBG_MP << "entering create mode" << std::endl;

	bool configure_canceled;
	bool connect_canceled;

	do {
		configure_canceled = false;
		connect_canceled = false;

		if (gui2::new_widgets) {

			gui2::tmp_create_game dlg(game_config);

			dlg.show(disp.video());

			network::send_data(config("refresh_lobby"), 0);
		} else {

			mp::ui::result res;
			mp_game_settings new_params;

			{
				mp::create ui(disp, game_config, state, gamechat, gamelist);
				run_lobby_loop(disp, ui);
				res = ui.get_result();
				new_params = ui.get_parameters();
			}

			switch (res) {
			case mp::ui::CREATE:
				configure_canceled = !enter_configure_mode(disp, game_config,
					state, new_params, local_players_only);
				break;
			case mp::ui::LOAD_GAME:
				connect_canceled = !enter_connect_mode(disp, game_config,
					state, new_params, local_players_only);
				break;
			case mp::ui::QUIT:
			default:
				//update lobby content
				network::send_data(config("refresh_lobby"), 0);
				break;
			}
		}
	} while(configure_canceled || connect_canceled);
}

static bool enter_configure_mode(game_display& disp, const config& game_config,
	game_state& state, const mp_game_settings& params, bool local_players_only)
{
	DBG_MP << "entering configure mode" << std::endl;

	bool connect_canceled;

	do {
		connect_canceled = false;

		mp::ui::result res;
		mp_game_settings new_params;

		{
			mp::configure ui(disp, game_config, gamechat, gamelist, params,
				local_players_only);
			run_lobby_loop(disp, ui);
			res = ui.get_result();
			new_params = ui.get_parameters();
		}

		switch (res) {
		case mp::ui::CREATE:
			connect_canceled = !enter_connect_mode(disp, game_config,
				state, new_params, local_players_only);
			break;
		case mp::ui::QUIT:
		default:
			//update lobby content
			network::send_data(config("refresh_lobby"), 0);
			return false;
		}
	} while(connect_canceled);

	return true;
}

static void do_preferences_dialog(game_display& disp, const config& game_config)
{
	DBG_MP << "displaying preferences dialog" << std::endl;
	const preferences::display_manager disp_manager(&disp);
	preferences::show_preferences_dialog(disp,game_config);

	/**
	 * The screen size might have changed force an update of the size.
	 *
	 * @todo This might no longer be needed when gui2 is done.
	 */
	const SDL_Rect rect = screen_area();
	preferences::set_resolution(disp.video(), rect.w, rect.h);

	gui2::settings::gamemap_width += rect.w - gui2::settings::screen_width ;
	gui2::settings::gamemap_height += rect.h - gui2::settings::screen_height ;
	gui2::settings::screen_width = rect.w;
	gui2::settings::screen_height = rect.h;
}

static void enter_lobby_mode(game_display& disp, const config& game_config,
	game_state& state)
{
	DBG_MP << "entering lobby mode" << std::endl;

	mp::ui::result res;

	while (true) {
		const config &cfg = game_config.child("lobby_music");
		if (cfg) {
			BOOST_FOREACH(const config &i, cfg.child_range("music")) {
				sound::play_music_config(i);
			}
			sound::commit_music_changes();
		} else {
			sound::empty_playlist();
			sound::stop_music();
		}
		lobby_info li(game_config);

		// Force a black background
		const Uint32 color = SDL_MapRGBA(disp.video().getSurface()->format
				, 0
				, 0
				, 0
				, 255);

		sdl_fill_rect(disp.video().getSurface(), NULL, color);

		if(preferences::new_lobby()) {
			gui2::tlobby_main dlg(game_config, li, disp);
			dlg.set_preferences_callback(
				boost::bind(do_preferences_dialog,
					boost::ref(disp), boost::ref(game_config)));
			dlg.show(disp.video());
			//ugly kludge for launching other dialogs like the old lobby
			switch (dlg.get_legacy_result()) {
				case gui2::tlobby_main::CREATE:
					res = mp::ui::CREATE;
					break;
				case gui2::tlobby_main::JOIN:
					res = mp::ui::JOIN;
					break;
				case gui2::tlobby_main::OBSERVE:
					res = mp::ui::OBSERVE;
					break;
				default:
					res = mp::ui::QUIT;
			}
		} else {
			mp::lobby ui(disp, game_config, gamechat, gamelist);
			run_lobby_loop(disp, ui);
			res = ui.get_result();
		}

		switch (res) {
		case mp::ui::JOIN:
			try {
				enter_wait_mode(disp, game_config, state, false);
			} catch(config::error& error) {
				if(!error.message.empty()) {
					gui2::show_error_message(disp.video(), error.message);
				}
				//update lobby content
				network::send_data(config("refresh_lobby"), 0);
			}
			break;
		case mp::ui::OBSERVE:
			try {
				enter_wait_mode(disp, game_config, state, true);
			} catch(config::error& error) {
				if(!error.message.empty()) {
					gui2::show_error_message(disp.video(), error.message);
				}
			}
			// update lobby content unconditionally because we might have left only after the
			// game ended in which case we ignored the gamelist and need to request it again
			network::send_data(config("refresh_lobby"), 0);
			break;
		case mp::ui::CREATE:
			try {
				enter_create_mode(disp, game_config, state, false);
			} catch(config::error& error) {
				if (!error.message.empty())
					gui2::show_error_message(disp.video(), error.message);
				//update lobby content
				network::send_data(config("refresh_lobby"), 0);
			}
			break;
		case mp::ui::QUIT:
			return;
		case mp::ui::PREFERENCES:
			{
				do_preferences_dialog(disp, game_config);
				//update lobby content
				network::send_data(config("refresh_lobby"), 0);
			}
			break;
		default:
			return;
		}
	}
}

namespace mp {

void start_local_game(game_display& disp, const config& game_config,
	game_state& state)
{
	DBG_MP << "starting local game" << std::endl;
	gamechat.clear_history();
	gamelist.clear();
	playmp_controller::set_replay_last_turn(0);
	preferences::set_message_private(false);
	enter_create_mode(disp, game_config, state, true);
}

void start_local_game_commandline(game_display& disp, const config& game_config,
	game_state& state, const commandline_options& cmdline_opts)
{
	DBG_MP << "starting local MP game from commandline" << std::endl;

	// The setup is done equivalently to lobby MP games using as much of existing
	// code as possible.  This means that some things are set up that are not
	// needed in commandline mode, but they are required by the functions called.
	gamechat.clear_history();
	gamelist.clear();
	playmp_controller::set_replay_last_turn(0);
	preferences::set_message_private(false);

	DBG_MP << "entering create mode" << std::endl;

	// Set the default parameters
	mp_game_settings parameters;  // This creates these parameters with default values defined in mp_game_settings.cpp

	// Hardcoded default values
	parameters.mp_era = "era_default";
	parameters.name = "multiplayer_The_Freelands";

	// Default values for which at getter function exists
	parameters.num_turns = settings::get_turns("");
	parameters.village_gold = settings::get_village_gold("");
	parameters.village_support = settings::get_village_support("");
	parameters.xp_modifier = settings::get_xp_modifier("");

	// Do not use map settings if --ignore-map-settings commandline option is set
	if(cmdline_opts.multiplayer_ignore_map_settings) {
		DBG_MP << "ignoring map settings" << std::endl;
		parameters.use_map_settings = false;
	} else {
		parameters.use_map_settings = true;
	}

	// We also want the following in order to be consistent with MP lobby mode:
	parameters.share_view = true;

	// None of the other parameters need to be set, as their creation values above are good enough for CL mode.
	// In particular, we do not want to use the preferences values.

	// Override era, faction (side) and scenario if set on the commandline
	if (cmdline_opts.multiplayer_era) parameters.mp_era = *cmdline_opts.multiplayer_era;
	const config &era_cfg = game_config.find_child("era", "id", parameters.mp_era);
	if (!era_cfg) {
		std::cerr << "Could not find era '" << parameters.mp_era << "'\n";
		return;
	}

	if (cmdline_opts.multiplayer_side) {
		for(std::vector<boost::tuple<unsigned int, std::string> >::const_iterator
			it=cmdline_opts.multiplayer_side->begin(); it!=cmdline_opts.multiplayer_side->end(); ++it)
		{
			const config *faction = &era_cfg.find_child("multiplayer_side", "id", it->get<1>());
			if (!*faction) {
				std::cerr << "Could not find faction '" << it->get<1>() << "'\n";
				return;
			}
		}
	}

	if (cmdline_opts.multiplayer_scenario) parameters.name = *cmdline_opts.multiplayer_scenario;
	const config &level = game_config.find_child("multiplayer", "id", parameters.name);
	if (!level) {
		std::cerr << "Could not find scenario '" << parameters.name << "'\n";
		return;
	}

	// Should the map be randomly generated?
	if (level["map_generation"].empty()) {
		DBG_MP << "using scenario map" << std::endl;
		parameters.scenario_data = level;
	} else {
		DBG_MP << "generating random map" << std::endl;
		util::scoped_ptr<map_generator> generator(NULL);
		generator.assign(create_map_generator(level["map_generation"], level.child("generator")));
		parameters.scenario_data = generator->create_scenario(std::vector<std::string>());

		// Set the scenario to have placing of sides
		// based on the terrain they prefer
		parameters.scenario_data["modify_placing"] = "true";

		util::unique_ptr<gamemap> map;
		const int map_positions = level.child("generator")["players"];
		DBG_MP << "map positions: " << map_positions << std::endl;

		for (int pos = parameters.scenario_data.child_count("side"); pos < map_positions; ++pos) {
			config& side = parameters.scenario_data.add_child("side");
			side["side"] = pos + 1;
			side["team_name"] = pos + 1;
			side["canrecruit"] = true;
			side["controller"] = "human";
		}
	}

	// Should number of turns be determined from scenario data?
	if (parameters.use_map_settings && parameters.scenario_data["turns"]) {
		DBG_MP << "setting turns from scenario data: " << parameters.scenario_data["turns"] << std::endl;
		parameters.num_turns = parameters.scenario_data["turns"];
	}

	DBG_MP << "entering connect mode" << std::endl;

	statistics::fresh_stats();

	{
		mp::connect_engine_ptr connect_engine(new mp::connect_engine(disp,
			state, parameters, true, true));
		mp::connect ui(disp, parameters.name, game_config, gamechat, gamelist,
			*connect_engine);

		// Update the parameters to reflect game start conditions
		ui.start_game_commandline(cmdline_opts);
	}

	std::string label = "";
	if (cmdline_opts.multiplayer_label) label = *cmdline_opts.multiplayer_label;
	recorder.add_log_data("ai_log","ai_label",label);

	unsigned int repeat = (cmdline_opts.multiplayer_repeat) ? *cmdline_opts.multiplayer_repeat : 1;
	for(unsigned int i = 0; i < repeat; i++){
		game_state state_copy(state);
		play_game(disp, state_copy, game_config, IO_SERVER, false, false);
	}
	recorder.clear();
}

void start_client(game_display& disp, const config& game_config,
	game_state& state, const std::string& host)
{
	DBG_MP << "starting client" << std::endl;
	const network::manager net_manager(1,1);

	gamechat.clear_history();
	gamelist.clear();
	server_type type = open_connection(disp, host);

	switch(type) {
	case WESNOTHD_SERVER:
		enter_lobby_mode(disp, game_config, state);
		break;
	case SIMPLE_SERVER:
		playmp_controller::set_replay_last_turn(0);
		preferences::set_message_private(false);
		enter_wait_mode(disp, game_config, state, false);
		break;
	case ABORT_SERVER:
		break;
	}
}

mp::ui::result goto_mp_connect(game_display& disp, connect_engine& engine,
	const config& game_config, const std::string& game_name)
{
	mp::ui::result res;

	{
		mp::connect ui(disp, game_name, game_config, gamechat, gamelist,
			engine);
		run_lobby_loop(disp, ui);

		res = ui.get_result();
		if (res == mp::ui::PLAY) {
			ui.start_game();
		}
	}

	return res;
}

mp::ui::result goto_mp_wait(game_state& state, game_display& disp,
	const config& game_config, bool observe)
{
	mp::ui::result res;

	{
		mp::wait ui(disp, game_config, state, gamechat, gamelist, false);

		ui.join_game(observe);
		run_lobby_loop(disp, ui);

		res = ui.get_result();
		if (res == mp::ui::PLAY) {
			ui.start_game();
		}
	}

	return res;
}

} // end namespace mp

