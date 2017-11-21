/*
   Copyright (C) 2007 - 2017 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "game_initialization/multiplayer.hpp"

#include "addon/manager.hpp" // for installed_addons
#include "events.hpp"
#include "formula/string_utils.hpp"
#include "game_config_manager.hpp"
#include "game_initialization/mp_game_utils.hpp"
#include "game_initialization/playcampaign.hpp"
#include "preferences/credentials.hpp"
#include "preferences/game.hpp"
#include "gettext.hpp"
#include "gui/dialogs/loading_screen.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/multiplayer/lobby.hpp"
#include "gui/dialogs/multiplayer/mp_create_game.hpp"
#include "gui/dialogs/multiplayer/mp_join_game.hpp"
#include "gui/dialogs/multiplayer/mp_login.hpp"
#include "gui/dialogs/multiplayer/mp_staging.hpp"
#include "gui/widgets/settings.hpp"
#include "hash.hpp"
#include "log.hpp"
#include "multiplayer_error_codes.hpp"
#include "settings.hpp"
#include "sound.hpp"
#include "statistics.hpp"
#include "wesnothd_connection.hpp"
#include "resources.hpp"
#include "replay.hpp"

#include "utils/functional.hpp"

static lg::log_domain log_mp("mp/main");
#define DBG_MP LOG_STREAM(debug, log_mp)

namespace
{
/** Opens a new server connection and prompts the client for login credentials, if necessary. */
std::pair<wesnothd_connection_ptr, config> open_connection(std::string host)
{
	DBG_MP << "opening connection" << std::endl;

	wesnothd_connection_ptr sock;
	if(host.empty()) {
		return std::make_pair(std::move(sock), config());
	}

	const int colon_index = host.find_first_of(":");
	unsigned int port;

	if(colon_index == -1) {
		port = 15000;
	} else {
		port = lexical_cast_default<unsigned int>(host.substr(colon_index + 1), 15000);
		host = host.substr(0, colon_index);
	}

	// shown_hosts is used to prevent the client being locked in a redirect loop.
	using hostpair = std::pair<std::string, int>;

	std::set<hostpair> shown_hosts;
	shown_hosts.emplace(host, port);

	// Initializes the connection to the server.
	sock = wesnothd_connection::create(host, std::to_string(port));
	if(!sock) {
		return std::make_pair(std::move(sock), config());
	}

	// Start stage
	gui2::dialogs::loading_screen::progress(loading_stage::connect_to_server);

	// First, spin until we get a handshake from the server.
	while(!sock->handshake_finished()) {
		sock->poll();
		SDL_Delay(1);
	}

	gui2::dialogs::loading_screen::progress(loading_stage::waiting);

	config data;
	config initial_lobby_config;

	bool received_join_lobby = false;
	bool received_gamelist = false;

	// Then, log in and wait for the lobby/game join prompt.
	do {
		if(!sock) {
			return std::make_pair(std::move(sock), config());
		}

		data.clear();
		sock->wait_and_receive_data(data);

		if(data.has_child("reject") || data.has_attribute("version")) {
			std::string version;

			if(const config& reject = data.child("reject")) {
				version = reject["accepted_versions"].str();
			} else {
				// Backwards-compatibility "version" attribute
				version = data["version"].str();
			}

			utils::string_map i18n_symbols;
			i18n_symbols["required_version"] = version;
			i18n_symbols["your_version"] = game_config::version;

			const std::string errorstring = vgettext("The server accepts versions '$required_version', but you are using version '$your_version'", i18n_symbols);
			throw wesnothd_error(errorstring);
		}

		// Check for "redirect" messages
		if(const config& redirect = data.child("redirect")) {
			host = redirect["host"].str();
			port = redirect["port"].to_int(15000);

			if(shown_hosts.find(hostpair(host, port)) != shown_hosts.end()) {
				throw wesnothd_error(_("Server-side redirect loop"));
			}

			shown_hosts.emplace(host, port);

			// Open a new connection with the new host and port.
			sock = wesnothd_connection_ptr();
			sock = wesnothd_connection::create(host, std::to_string(port));
			continue;
		}

		if(data.has_child("version")) {
			config cfg;
			config res;
			cfg["version"] = game_config::version;
			res.add_child("version", cfg);
			sock->send_data(res);
		}

		// Check for gamelist. This *must* be done before the mustlogin check
		// or else this loop will run ad-infinitum.
		if(data.has_child("gamelist")) {
			received_gamelist = true;

			// data should only contain the game and user lists at this point, so just swap it.
			std::swap(initial_lobby_config, data);
		}

		// Continue if we did not get a direction to login
		if(!data.has_child("mustlogin")) {
			continue;
		}

		// Enter login loop
		for(;;) {
			std::string password_reminder = "";

			std::string login = preferences::login();

			config response ;
			config& sp = response.add_child("login") ;
			sp["username"] = login ;

			// Login and enable selective pings -- saves server bandwidth
			// If ping_timeout has a non-zero value, do not enable
			// selective pings as this will cause clients to falsely
			// believe the server has died and disconnect.
			if(preferences::get_ping_timeout()) {
				// Pings required so disable selective pings
				sp["selective_ping"] = false;
			} else {
				// Client is bandwidth friendly so allow
				// server to optimize ping frequency as needed.
				sp["selective_ping"] = true;
			}

			sock->send_data(response);
			sock->wait_and_receive_data(data);

			gui2::dialogs::loading_screen::progress(loading_stage::login_response);

			config* warning = &data.child("warning");

			if(*warning) {
				std::string warning_msg;

				if((*warning)["warning_code"] == MP_NAME_INACTIVE_WARNING) {
					warning_msg = vgettext("The nickname ‘$nick’ is inactive. "
						"You cannot claim ownership of this nickname until you "
						"activate your account via email or ask an "
						"administrator to do it for you.", {{"nick", login}});
				} else {
					warning_msg = (*warning)["message"].str();
				}

				warning_msg += "\n\n";
				warning_msg += _("Do you want to continue?");

				if(gui2::show_message(_("Warning"), warning_msg, gui2::dialogs::message::yes_no_buttons) != gui2::window::OK) {
					return std::make_pair(wesnothd_connection_ptr(), config());
				}
			}

			config* error = &data.child("error");

			// ... and get us out of here if the server did not complain
			if(!*error) break;

			do {
				std::string password = preferences::password(host, login);

				bool fall_through = (*error)["force_confirmation"].to_bool() ?
					(gui2::show_message(_("Confirm"), (*error)["message"], gui2::dialogs::message::ok_cancel_buttons) == gui2::window::CANCEL) :
					false;

				const bool is_pw_request = !((*error)["password_request"].empty()) && !(password.empty());

				// If the server asks for a password, provide one if we can
				// or request a password reminder.
				// Otherwise or if the user pressed 'cancel' in the confirmation dialog
				// above go directly to the username/password dialog
				if((is_pw_request || !password_reminder.empty()) && !fall_through) {
					if(is_pw_request) {
						if((*error)["phpbb_encryption"].to_bool()) {
							// Apparently HTML key-characters are passed to the hashing functions of phpbb in this escaped form.
							// I will do closer investigations on this, for now let's just hope these are all of them.

							// Note: we must obviously replace '&' first, I wasted some time before I figured that out... :)
							for(std::string::size_type pos = 0; (pos = password.find('&', pos)) != std::string::npos; ++pos)
								password.replace(pos, 1, "&amp;");
							for(std::string::size_type pos = 0; (pos = password.find('\"', pos)) != std::string::npos; ++pos)
								password.replace(pos, 1, "&quot;");
							for(std::string::size_type pos = 0; (pos = password.find('<', pos)) != std::string::npos; ++pos)
								password.replace(pos, 1, "&lt;");
							for(std::string::size_type pos = 0; (pos = password.find('>', pos)) != std::string::npos; ++pos)
								password.replace(pos, 1, "&gt;");

							const std::string salt = (*error)["salt"];

							if(salt.length() < 12) {
								throw wesnothd_error(_("Bad data received from server"));
							}

							sp["password"] = utils::md5(utils::md5(password, utils::md5::get_salt(salt),
									utils::md5::get_iteration_count(salt)).base64_digest(), salt.substr(12, 8)).base64_digest();

						} else {
							sp["password"] = password;
						}
					}

					sp["password_reminder"] = password_reminder;

					// Once again send our request...
					sock->send_data(response);
					sock->wait_and_receive_data(data);

					gui2::dialogs::loading_screen::progress(loading_stage::login_response);

					error = &data.child("error");

					// ... and get us out of here if the server is happy now
					if(!*error) break;
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
							"characters. Only alpha-numeric characters (one at minimum), underscores and "
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

				gui2::dialogs::mp_login dlg(host, error_message, !((*error)["password_request"].empty()));

				// Need to show the dialog from the main thread or it won't appear.
				events::call_in_main_thread([&dlg]() { dlg.show(); });

				switch(dlg.get_retval()) {
					//Log in with password
					case gui2::window::OK:
						break;
					//Request a password reminder
					case 1:
						password_reminder = "yes";
						break;
					// Cancel
					default:
						return std::make_pair(wesnothd_connection_ptr(), config());
				}

			// If we have got a new username we have to start all over again
			} while(login == preferences::login());

			// Somewhat hacky...
			// If we broke out of the do-while loop above error
			// is still going to be nullptr
			if(!*error) break;
		} // end login loop

		if(data.has_child("join_lobby")) {
			received_join_lobby = true;

			gui2::dialogs::loading_screen::progress(loading_stage::download_lobby_data);
		}
	} while(!received_join_lobby || !received_gamelist);

	return std::make_pair(std::move(sock), std::move(initial_lobby_config));
}

/** Helper struct to manage the MP workflow arguments. */
struct mp_workflow_helper
{
	mp_workflow_helper(const config& gc, saved_game& state, wesnothd_connection* connection, mp::lobby_info* li)
		: game_config(gc)
		, state(state)
		, connection(connection)
		, lobby_info(li)
	{}

	const config& game_config;

	saved_game& state;

	wesnothd_connection* connection;

	mp::lobby_info* lobby_info;
};

using mp_workflow_helper_ptr = std::shared_ptr<mp_workflow_helper>;

/**
 * The main components of the MP workflow. It consists of four screens:
 *
 * Host POV:   LOBBY <---> CREATE GAME ---> STAGING ------------------> GAME BEGINS
 * Player POV: LOBBY <---------------------------------> JOIN GAME ---> GAME BEGINS
 *
 * NOTE: since these functions are static, they appear here in the opposite order they'd be accessed.
 */
void enter_wait_mode(mp_workflow_helper_ptr helper, int game_id, bool observe)
{
	DBG_MP << "entering wait mode" << std::endl;

	// The connection should never be null here, since one should never reach this screen in local game mode.
	assert(helper->connection);

	statistics::fresh_stats();

	std::unique_ptr<mp_campaign_info> campaign_info(new mp_campaign_info(*helper->connection));
	campaign_info->is_host = false;

	if(helper->lobby_info->get_game_by_id(game_id)) {
		campaign_info->current_turn = helper->lobby_info->get_game_by_id(game_id)->current_turn;
	}

	if(preferences::skip_mp_replay() || preferences::blindfold_replay()) {
		campaign_info->skip_replay = true;
		campaign_info->skip_replay_blindfolded = preferences::blindfold_replay();
	}

	bool dlg_ok = false;
	{
		gui2::dialogs::mp_join_game dlg(helper->state, *helper->lobby_info, *helper->connection, true, observe);

		if(!dlg.fetch_game_config()) {
			return;
		}

		dlg.show();
		dlg_ok = dlg.get_retval() == gui2::window::OK;
	}

	if(dlg_ok) {
		campaign_controller controller(helper->state, helper->game_config, game_config_manager::get()->terrain_types());
		controller.set_mp_info(campaign_info.get());
		controller.play_game();
	}

	helper->connection->send_data(config("leave_game"));
}

void enter_staging_mode(mp_workflow_helper_ptr helper)
{
	DBG_MP << "entering connect mode" << std::endl;

	std::unique_ptr<mp_campaign_info> campaign_info;

	// If we have a connection, set the appropriate info. No connection means we're in local game mode.
	if(helper->connection) {
		campaign_info.reset(new mp_campaign_info(*helper->connection));
		campaign_info->connected_players.insert(preferences::login());
		campaign_info->is_host = true;
	}

	bool dlg_ok = false;
	{
		ng::connect_engine_ptr connect_engine(new ng::connect_engine(helper->state, true, campaign_info.get()));

		gui2::dialogs::mp_staging dlg(*connect_engine, *helper->lobby_info, helper->connection);
		dlg.show();
		dlg_ok = dlg.get_retval() == gui2::window::OK;
	} // end connect_engine_ptr, dlg scope

	if(dlg_ok) {
		campaign_controller controller(helper->state, helper->game_config, game_config_manager::get()->terrain_types());
		controller.set_mp_info(campaign_info.get());
		controller.play_game();
	}

	if(helper->connection) {
		helper->connection->send_data(config("leave_game"));
	}
}

void enter_create_mode(mp_workflow_helper_ptr helper)
{
	DBG_MP << "entering create mode" << std::endl;

	bool dlg_cancel = false;
	{
		bool local_mode = helper->connection == nullptr;
		mp::user_info* host_info = helper->lobby_info->get_user(preferences::login());

		gui2::dialogs::mp_create_game dlg(helper->game_config, helper->state, local_mode, host_info);
		dlg_cancel = !dlg.show();
	}

	if(!dlg_cancel) {
		enter_staging_mode(helper);
	} else if(helper->connection) {
		helper->connection->send_data(config("refresh_lobby"));
	}
}

bool enter_lobby_mode(mp_workflow_helper_ptr helper, const std::vector<std::string>& installed_addons, const config& initial_lobby_config)
{
	DBG_MP << "entering lobby mode" << std::endl;

	// Connection should never be null in the lobby.
	assert(helper->connection);

	// We use a loop here to allow returning to the lobby if you, say, cancel game creation.
	while(true) {
		if(const config& cfg = helper->game_config.child("lobby_music")) {
			for(const config& i : cfg.child_range("music")) {
				sound::play_music_config(i);
			}

			sound::commit_music_changes();
		} else {
			sound::empty_playlist();
			sound::stop_music();
		}

		mp::lobby_info li(helper->game_config, installed_addons);
		helper->lobby_info = &li;

		if(!initial_lobby_config.empty()) {
			li.process_gamelist(initial_lobby_config);
		}

		int dlg_retval = 0;
		int dlg_joined_game_id = 0;
		{

			gui2::dialogs::mp_lobby dlg(helper->game_config, li, *helper->connection);
			dlg.show();
			dlg_retval = dlg.get_retval();
			dlg_joined_game_id = dlg.get_joined_game_id();
		}

		switch(dlg_retval) {
			case gui2::dialogs::mp_lobby::CREATE:
				try {
					enter_create_mode(helper);
				} catch(config::error& error) {
					if(!error.message.empty()) {
						gui2::show_error_message(error.message);
					}

					// Update lobby content
					helper->connection->send_data(config("refresh_lobby"));
				}

				break;
			case gui2::dialogs::mp_lobby::JOIN:
			case gui2::dialogs::mp_lobby::OBSERVE:
				try {
					enter_wait_mode(helper,
						dlg_joined_game_id,
						dlg_retval == gui2::dialogs::mp_lobby::OBSERVE
					);
				} catch(config::error& error) {
					if(!error.message.empty()) {
						gui2::show_error_message(error.message);
					}

					// Update lobby content
					helper->connection->send_data(config("refresh_lobby"));
				}

				break;
			case gui2::dialogs::mp_lobby::RELOAD_CONFIG:
				// Let this function's caller reload the config and re-call.
				return false;
			default:
				// Needed to handle the Quit signal and exit the loop
				return true;
		}
	}

	return true;
}

} // end anon namespace

/** Pubic entry points for the MP workflow */
namespace mp
{
void start_client(const config& game_config,	saved_game& state, const std::string& host)
{
	const config* game_config_ptr = &game_config;

	// This function does not refer to an addon database, it calls filesystem functions.
	// For the sanity of the mp lobby, this list should be fixed for the entire lobby session,
	// even if the user changes the contents of the addon directory in the meantime.
	std::vector<std::string> installed_addons = ::installed_addons();

	DBG_MP << "starting client" << std::endl;

	preferences::admin_authentication_reset r;

	wesnothd_connection_ptr connection;
	config lobby_config;

	gui2::dialogs::loading_screen::display([&]() {
		std::tie(connection, lobby_config) = open_connection(host);
	});

	if(!connection) {
		return;
	}

	mp_workflow_helper_ptr workflow_helper;
	bool re_enter = false;

	do {
		workflow_helper.reset(new mp_workflow_helper(*game_config_ptr, state, connection.get(), nullptr));

		// A return of false means a config reload was requested, so do that and then loop.
		re_enter = !enter_lobby_mode(workflow_helper, installed_addons, lobby_config);

		if(re_enter) {
			game_config_manager* gcm = game_config_manager::get();
			gcm->reload_changed_game_config();
			gcm->load_game_config_for_game(state.classification()); // NOTE: Using reload_changed_game_config only doesn't seem to work here

			game_config_ptr = &gcm->game_config();

			installed_addons = ::installed_addons(); // Refresh the installed add-on list for this session.

			connection->send_data(config("refresh_lobby"));
		}
	} while(re_enter);
}

bool goto_mp_connect(ng::connect_engine& engine, const config& game_config, wesnothd_connection* connection)
{
	lobby_info li(game_config, {});

	gui2::dialogs::mp_staging dlg(engine, li, connection);
	return dlg.show();
}

bool goto_mp_wait(saved_game& state, const config& game_config, wesnothd_connection* connection, bool observe)
{
	lobby_info li(game_config, std::vector<std::string>());

	gui2::dialogs::mp_join_game dlg(state, li, *connection, false, observe);

	if(!dlg.fetch_game_config()) {
		return false;
	}

	if(dlg.started()) {
		return true;
	}

	return dlg.show();
}

void start_local_game(const config& game_config, saved_game& state)
{
	DBG_MP << "starting local game" << std::endl;

	preferences::set_message_private(false);

	// TODO: should lobby_info take a nullptr in this case, or should we pass the installed_addons data here too?
	lobby_info li(game_config, {});
	mp_workflow_helper_ptr workflow_helper = std::make_shared<mp_workflow_helper>(game_config, state, nullptr, &li);

	enter_create_mode(workflow_helper);
}

void start_local_game_commandline(const config& game_config, saved_game& state, const commandline_options& cmdline_opts)
{
	DBG_MP << "starting local MP game from commandline" << std::endl;

	// The setup is done equivalently to lobby MP games using as much of existing
	// code as possible.  This means that some things are set up that are not
	// needed in commandline mode, but they are required by the functions called.
	preferences::set_message_private(false);

	DBG_MP << "entering create mode" << std::endl;

	// Set the default parameters
	state.clear(); // This creates these parameters with default values defined in mp_game_settings.cpp
	mp_game_settings& parameters = state.mp_settings();

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

	// None of the other parameters need to be set, as their creation values above are good enough for CL mode.
	// In particular, we do not want to use the preferences values.

	state.classification().campaign_type = game_classification::CAMPAIGN_TYPE::MULTIPLAYER;

	// [era] define.
	if(cmdline_opts.multiplayer_era) {
		parameters.mp_era = *cmdline_opts.multiplayer_era;
	}

	if(const config& cfg_era = game_config.find_child("era", "id", parameters.mp_era)) {
		state.classification().era_define = cfg_era["define"].str();
	} else {
		std::cerr << "Could not find era '" << parameters.mp_era << "'\n";
		return;
	}

	// [multiplayer] define.
	if(cmdline_opts.multiplayer_scenario) {
		parameters.name = *cmdline_opts.multiplayer_scenario;
	}

	if(const config& cfg_multiplayer = game_config.find_child("multiplayer", "id", parameters.name)) {
		state.classification().scenario_define = cfg_multiplayer["define"].str();
	} else {
		std::cerr << "Could not find [multiplayer] '" << parameters.name << "'\n";
		return;
	}

	game_config_manager::get()->load_game_config_for_game(state.classification());
	state.set_carryover_sides_start(
		config {"next_scenario", parameters.name}
	);

	state.expand_random_scenario();
	state.expand_mp_events();
	state.expand_mp_options();

	// Should number of turns be determined from scenario data?
	if(parameters.use_map_settings && state.get_starting_pos()["turns"]) {
		DBG_MP << "setting turns from scenario data: " << state.get_starting_pos()["turns"] << std::endl;
		parameters.num_turns = state.get_starting_pos()["turns"];
	}

	DBG_MP << "entering connect mode" << std::endl;

	statistics::fresh_stats();

	{
		ng::connect_engine_ptr connect_engine(new ng::connect_engine(state, true, nullptr));

		// Update the parameters to reflect game start conditions
		connect_engine->start_game_commandline(cmdline_opts);
	}

	if(resources::recorder && cmdline_opts.multiplayer_label) {
		std::string label = *cmdline_opts.multiplayer_label;
		resources::recorder->add_log_data("ai_log","ai_label",label);
	}

	unsigned int repeat = (cmdline_opts.multiplayer_repeat) ? *cmdline_opts.multiplayer_repeat : 1;
	for(unsigned int i = 0; i < repeat; i++){
		saved_game state_copy(state);
		campaign_controller controller(state_copy, game_config, game_config_manager::get()->terrain_types());
		controller.play_game();
	}
}

} // end namespace mp
