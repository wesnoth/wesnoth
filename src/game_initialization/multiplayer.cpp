/*
   Copyright (C) 2007 - 2016 by David White <dave@whitevine.net>
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

#include "addon/manager.hpp" // for get_installed_addons
#include "config_assign.hpp"
#include "font/sdl_ttf.hpp"
#include "formula/string_utils.hpp"
#include "game_preferences.hpp"
#include "generators/map_create.hpp"
#include "generators/map_generator.hpp"
#include "gettext.hpp"
#include "gui/dialogs/loadscreen.hpp"
#include "gui/dialogs/lobby/lobby.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/multiplayer/mp_connect.hpp"
#include "gui/dialogs/multiplayer/mp_create_game.hpp"
#include "gui/dialogs/multiplayer/mp_join_game.hpp"
#include "gui/dialogs/multiplayer/mp_login.hpp"
#include "gui/dialogs/multiplayer/mp_staging.hpp"
#include "gui/dialogs/network_transmission.hpp"
#include "gui/dialogs/preferences_dialog.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "hash.hpp"
#include "lobby_reload_request_exception.hpp"
#include "log.hpp"
#include "generators/map_create.hpp"
#include "mp_game_utils.hpp"
#include "multiplayer_configure.hpp"
#include "configure_engine.hpp"
#include "multiplayer_connect.hpp"
#include "multiplayer_create.hpp"
#include "multiplayer_error_codes.hpp"
#include "multiplayer_wait.hpp"
#include "multiplayer_lobby.hpp"
#include "playcampaign.hpp"
#include "settings.hpp"
#include "scripting/plugins/context.hpp"
#include "sdl/rect.hpp"
#include "sound.hpp"
#include "statistics.hpp"
#include "units/id.hpp"
#include "video.hpp"
#include "wesnothd_connection.hpp"
#include "game_config_manager.hpp"

#include "utils/functional.hpp"

static lg::log_domain log_network("network");
#define LOG_NW LOG_STREAM(info, log_network)

static lg::log_domain log_mp("mp/main");
#define DBG_MP LOG_STREAM(debug, log_mp)

namespace {

mp::chat gamechat;
config gamelist;

}

namespace mp {

void run_lobby_loop(CVideo& video, mp::ui& ui)
{
	DBG_MP << "running lobby loop" << std::endl;
	video.modeChanged();
	bool first = true;
	font::cache_mode(font::CACHE_LOBBY);
	while (ui.get_result() == mp::ui::CONTINUE) {
		if (video.modeChanged() || first) {
			SDL_Rect lobby_pos = sdl::create_rect(0
					, 0
					, video.getx()
					, video.gety());
			ui.set_location(lobby_pos);
			first = false;
		}
		// process network data first so user actions can override the result
		// or uptodate data can prevent invalid actions
		// i.e. press cancel while you receive [start_game] or press start game while someone leaves
		ui.process_network();

		if (plugins_context * pc = ui.get_plugins_context()) {
			pc->play_slice();
			//DBG_MP << "* playing a plugins slice\n";
		}

		events::pump();
		events::raise_process_event();
		events::raise_draw_event();

		video.flip();
		CVideo::delay(20);
	}
	font::cache_mode(font::CACHE_GAME);
}

}


static std::unique_ptr<twesnothd_connection> open_connection(CVideo& video, const std::string& original_host)
{
	DBG_MP << "opening connection" << std::endl;
	std::string h = original_host;

	if(h.empty()) {
		gui2::tmp_connect dlg;

		dlg.show(video);
		if(dlg.get_retval() == gui2::twindow::OK) {
			h = preferences::network_host();
		} else {
			return 0;
		}
	}
	std::unique_ptr<twesnothd_connection> sock;

	const int colon_index = h.find_first_of(":");
	std::string host;
	unsigned int port;

	if(colon_index == -1) {
		host = h;
		port = 15000;
	} else {
		host = h.substr(0, colon_index);
		port = lexical_cast_default<unsigned int>(h.substr(colon_index + 1), 15000);
	}

	// shown_hosts is used to prevent the client being locked in a redirect
	// loop.
	typedef std::pair<std::string, int> hostpair;
	std::set<hostpair> shown_hosts;
	shown_hosts.insert(hostpair(host, port));

	config data;
	sock = gui2::tnetwork_transmission::wesnothd_connect_dialog(video, "connect to server", host, port);
	do {

		if (!sock) {
			return sock;
		}

		data.clear();
		gui2::tnetwork_transmission::wesnothd_receive_dialog(video, "waiting", data, *sock);
		//mp::check_response(data_res, data);

		if (data.has_child("reject") || data.has_attribute("version")) {
			std::string version;
			if (const config &reject = data.child("reject")) {
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
		if (const config &redirect = data.child("redirect"))
		{
			host = redirect["host"].str();
			port =redirect["port"].to_int(15000);

			if(shown_hosts.find(hostpair(host,port)) != shown_hosts.end()) {
				throw wesnothd_error(_("Server-side redirect loop"));
			}
			shown_hosts.insert(hostpair(host, port));
			sock.release();
			sock = gui2::tnetwork_transmission::wesnothd_connect_dialog(video, "redirect", host, port);
			continue;
		}

		if(data.child("version")) {
			config cfg;
			config res;
			cfg["version"] = game_config::version;
			res.add_child("version", cfg);
			sock->send_data(res);
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
				sock->send_data(response);
				gui2::tnetwork_transmission::wesnothd_receive_dialog(video, "login response", data, *sock);
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

					if(gui2::show_message(video, _("Warning"), warning_msg, gui2::tmessage::yes_no_buttons) != gui2::twindow::OK) {
						return 0;
					}
				}

				config *error = &data.child("error");

				// ... and get us out of here if the server did not complain
				if (!*error) break;

				do {
					std::string password = preferences::password();

					bool fall_through = (*error)["force_confirmation"].to_bool() ?
						(gui2::show_message(video, _("Confirm"), (*error)["message"], gui2::tmessage::ok_cancel_buttons) == gui2::twindow::CANCEL) :
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
									throw wesnothd_error(_("Bad data received from server"));
								}

								sp["password"] = util::create_hash(util::create_hash(password, util::get_salt(salt),
										util::get_iteration_count(salt)), salt.substr(12, 8));

							} else {
								sp["password"] = password;
							}
						}

						sp["password_reminder"] = password_reminder;

						// Once again send our request...
						sock->send_data(response);
						gui2::tnetwork_transmission::wesnothd_receive_dialog(video, "login response", data, *sock);


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
					dlg.show(video);

					switch(dlg.get_retval()) {
						//Log in with password
						case gui2::twindow::OK:
							break;
						//Request a password reminder
						case 1:
							password_reminder = "yes";
							break;
						// Cancel
						default:
							return 0;
					}

				// If we have got a new username we have to start all over again
				} while(login == preferences::login());

				// Somewhat hacky...
				// If we broke out of the do-while loop above error
				// is still going to be nullptr
				if(!*error) break;
			} // end login loop
		}
	} while(!(data.child("join_lobby") || data.child("join_game")));

	if (h != preferences::server_list().front().address)
		preferences::set_network_host(h);

	if (data.child("join_lobby")) {
		return sock;
	} else {
		return 0;
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
static void enter_wait_mode(CVideo& video, const config& game_config, saved_game& state, twesnothd_connection* wesnothd_connection,
	lobby_info& li, bool observe, int current_turn = 0)
{
	DBG_MP << "entering wait mode" << std::endl;

	mp::ui::result res;

	gamelist.clear();
	statistics::fresh_stats();
	std::unique_ptr<mp_campaign_info> campaign_info;
	campaign_info.reset(new mp_campaign_info(*wesnothd_connection));
	campaign_info->is_host = false;
	if(preferences::skip_mp_replay() || preferences::blindfold_replay()) {
		campaign_info->skip_replay_until_turn = current_turn;
		campaign_info->skip_replay_blindfolded = preferences::blindfold_replay();
	}

	{
		if(preferences::new_lobby()) {

			gui2::tmp_join_game dlg(state, li, *wesnothd_connection, true, observe);

			if(!dlg.fetch_game_config(video)) {
				return;
			}

			dlg.show(video);

			if(dlg.get_retval() == gui2::twindow::OK) {
				campaign_controller controller(video, state, game_config, game_config_manager::get()->terrain_types());
				controller.set_mp_info(campaign_info.get());
				controller.play_game();
			}

			if(wesnothd_connection) {
				wesnothd_connection->send_data(config("leave_game"));
			}

			return;
		}

		mp::wait ui(video, wesnothd_connection, game_config, state, gamechat, gamelist);

		ui.join_game(observe);

		run_lobby_loop(video, ui);
		res = ui.get_result();
		campaign_info->connected_players.insert(ui.user_list().begin(), ui.user_list().end());

		if (res == mp::ui::PLAY) {
			ui.start_game();
		} else {
			DBG_MP << "skipped wait mode, result = " << res << std::endl;
		}
	}

	switch (res) {
	case mp::ui::PLAY: {
		campaign_controller controller(video, state, game_config, game_config_manager::get()->terrain_types());
		controller.set_mp_info(campaign_info.get());
		controller.play_game();
		break;
	}
	case mp::ui::QUIT:
	default:
		break;
	}
	wesnothd_connection->send_data(config("leave_game"));
}

static void enter_create_mode(CVideo& video, const config& game_config, saved_game& state, twesnothd_connection* wesnothd_connection,
	lobby_info& li, bool local_players_only = false);

static bool enter_connect_mode(CVideo& video, const config& game_config,
	saved_game& state, twesnothd_connection* wesnothd_connection, lobby_info& li,
	bool local_players_only = false)
{
	DBG_MP << "entering connect mode" << std::endl;

	mp::ui::result res;

	gamelist.clear();
	statistics::fresh_stats();
	std::unique_ptr<mp_campaign_info> campaign_info;
	if(!local_players_only) {
		assert(wesnothd_connection);
		campaign_info.reset(new mp_campaign_info(*wesnothd_connection));
		campaign_info->connected_players.insert(preferences::login());
		campaign_info->is_host = true;
	}

	{
		ng::connect_engine_ptr connect_engine(new ng::connect_engine(state, true, campaign_info.get()));

		if(preferences::new_lobby()) {
			gui2::tmp_staging dlg(*connect_engine, li, wesnothd_connection);
			dlg.show(video);

			if(dlg.get_retval() == gui2::twindow::OK) {
				campaign_controller controller(video, state, game_config, game_config_manager::get()->terrain_types());
				controller.set_mp_info(campaign_info.get());
				controller.play_game();
			}

			if(wesnothd_connection) {
				wesnothd_connection->send_data(config("leave_game"));
			}

			return true;
		}

		mp::connect ui(video, wesnothd_connection, state.mp_settings().name, game_config, gamechat, gamelist,
			*connect_engine);
		run_lobby_loop(video, ui);

		res = ui.get_result();

		// start_game() updates the parameters to reflect game start,
		// so it must be called before get_level()
		if (res == mp::ui::PLAY) {
			ui.start_game();
		}
	} // end connect_engine_ptr scope

	switch (res) {
	case mp::ui::PLAY: {
		campaign_controller controller(video, state, game_config, game_config_manager::get()->terrain_types());
		controller.set_mp_info(campaign_info.get());
		controller.play_game();
		if (wesnothd_connection) {
			wesnothd_connection->send_data(config("leave_game"));
		}
		break;
	}
	case mp::ui::CREATE:
		enter_create_mode(video, game_config, state, wesnothd_connection, li, local_players_only);
		if (wesnothd_connection) {
			wesnothd_connection->send_data(config("leave_game"));
		}
		break;
	case mp::ui::QUIT:
	default:
		if (wesnothd_connection) {
			wesnothd_connection->send_data(config("refresh_lobby"));
			wesnothd_connection->send_data(config("leave_game"));
		}
		return false;
	}

	return true;
}

static bool enter_configure_mode(CVideo& video, const config& game_config, saved_game& state, twesnothd_connection* wesnothd_connection, lobby_info& li,
	bool local_players_only = false);

static void enter_create_mode(CVideo& video, const config& game_config,
	saved_game& state, twesnothd_connection* wesnothd_connection, lobby_info& li, bool local_players_only)
{
	DBG_MP << "entering create mode" << std::endl;

	bool configure_canceled;
	bool connect_canceled;

	if(preferences::new_lobby()) {
		ng::create_engine create_eng(video, state);

		gui2::tmp_create_game dlg(game_config, create_eng);

		dlg.show(video);

		if(dlg.get_retval() != gui2::twindow::CANCEL) {
			enter_connect_mode(video, game_config, state, wesnothd_connection, li, local_players_only);
		} else if(wesnothd_connection) {
			wesnothd_connection->send_data(config("refresh_lobby"));
		}

		return;
	}

	do {
		configure_canceled = false;
		connect_canceled = false;

		mp::ui::result res;

		{
			mp::create ui(video, wesnothd_connection, game_config, state, gamechat, gamelist);
			run_lobby_loop(video, ui);
			res = ui.get_result();
			ui.get_parameters();
		}

		switch (res) {
		case mp::ui::CREATE:
			configure_canceled = !enter_configure_mode(video, game_config, state, wesnothd_connection, li, local_players_only);
			break;
		case mp::ui::LOAD_GAME:
			connect_canceled = !enter_connect_mode(video, game_config, state, wesnothd_connection, li, local_players_only);
			break;
		case mp::ui::QUIT:
		default:
			//update lobby content
			if (wesnothd_connection) {
				wesnothd_connection->send_data(config("refresh_lobby"));
			}
			break;
		}
	} while(configure_canceled || connect_canceled);
}

static bool enter_configure_mode(CVideo& video, const config& game_config,
	saved_game& state, twesnothd_connection* wesnothd_connection, lobby_info& li, bool local_players_only)
{
	DBG_MP << "entering configure mode" << std::endl;

	bool connect_canceled;

	do {
		connect_canceled = false;

		mp::ui::result res;

		{
			if (state.get_starting_pos().child("side")) {
				mp::configure ui(video, wesnothd_connection, game_config, gamechat, gamelist, state,
					local_players_only);
				run_lobby_loop(video, ui);
				res = ui.get_result();
				ui.get_parameters();
			} else {
				gui2::show_error_message(video, _("No sides found. At least one side or starting position must be defined."));
				res = mp::ui::QUIT;
			}
		}

		switch (res) {
		case mp::ui::CREATE:
			connect_canceled = !enter_connect_mode(video, game_config, state, wesnothd_connection, li, local_players_only);
			break;
		case mp::ui::QUIT:
		default:
			//update lobby content
			if (wesnothd_connection) {
				wesnothd_connection->send_data(config("refresh_lobby"));
			}
			return false;
		}
	} while(connect_canceled);

	return true;
}

static void do_preferences_dialog(CVideo& video, const config& game_config)
{
	DBG_MP << "displaying preferences dialog" << std::endl;
	gui2::tpreferences::display(video, game_config);

	/**
	 * The screen size might have changed force an update of the size.
	 *
	 * @todo This might no longer be needed when gui2 is done.
	 */
	const SDL_Rect rect = screen_area();

	video.set_resolution(rect.w, rect.h);

	gui2::settings::gamemap_width += rect.w - gui2::settings::screen_width ;
	gui2::settings::gamemap_height += rect.h - gui2::settings::screen_height ;
	gui2::settings::screen_width = rect.w;
	gui2::settings::screen_height = rect.h;
}

static void enter_lobby_mode(CVideo& video, const config& game_config,
	saved_game& state, twesnothd_connection* wesnothd_connection, const std::vector<std::string> & installed_addons)
{
	assert(wesnothd_connection);
	DBG_MP << "entering lobby mode" << std::endl;

	mp::ui::result res;
	int current_turn = 0;
	while (true) {
		const config &cfg = game_config.child("lobby_music");
		if (cfg) {
			for (const config &i : cfg.child_range("music")) {
				sound::play_music_config(i);
			}
			sound::commit_music_changes();
		} else {
			sound::empty_playlist();
			sound::stop_music();
		}
		lobby_info li(game_config, installed_addons);

		// Force a black background
		const Uint32 color = SDL_MapRGBA(video.getSurface()->format
				, 0
				, 0
				, 0
				, SDL_ALPHA_OPAQUE);

		sdl::fill_rect(video.getSurface(), nullptr, color);

		if(preferences::new_lobby()) {
			gui2::tlobby_main dlg(game_config, li, *wesnothd_connection);
			dlg.set_preferences_callback(
				std::bind(do_preferences_dialog,
					std::ref(video), std::ref(game_config)));
			dlg.show(video);
			//ugly kludge for launching other dialogs like the old lobby
			switch(dlg.get_retval()) {
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
			mp::lobby ui(video, wesnothd_connection, game_config, gamechat, gamelist, installed_addons);
			run_lobby_loop(video, ui);
			res = ui.get_result();
			current_turn = ui.current_turn;
		}

		switch (res) {
		case mp::ui::JOIN:
		case mp::ui::OBSERVE:
			try {
				enter_wait_mode(video, game_config, state, wesnothd_connection, li, res == mp::ui::OBSERVE, current_turn);
			} catch(config::error& error) {
				if(!error.message.empty()) {
					gui2::show_error_message(video, error.message);
				}
				//update lobby content
				wesnothd_connection->send_data(config("refresh_lobby"));
			}
			break;
		case mp::ui::CREATE:
			try {
				enter_create_mode(video, game_config, state, wesnothd_connection, li, false);
			} catch(config::error& error) {
				if (!error.message.empty())
					gui2::show_error_message(video, error.message);
				//update lobby content
				wesnothd_connection->send_data(config("refresh_lobby"));
			}
			break;
		case mp::ui::QUIT:
			return;
		case mp::ui::PREFERENCES:
			{
				do_preferences_dialog(video, game_config);
				//update lobby content
				wesnothd_connection->send_data(config("refresh_lobby"));
			}
			break;
		default:
			return;
		}
	}
}

namespace mp {

void start_local_game(CVideo& video, const config& game_config,
	saved_game& state)
{
	DBG_MP << "starting local game" << std::endl;
	gamechat.clear_history();
	gamelist.clear();
	preferences::set_message_private(false);
	// TODO: should lobby_info take a nullptr in this case, or should we pass the installed_addons data here too?
	lobby_info li(game_config, std::vector<std::string>());
	enter_create_mode(video, game_config, state, nullptr, li, true);
}

void start_local_game_commandline(CVideo& video, const config& game_config,
	saved_game& state, const commandline_options& cmdline_opts)
{
	DBG_MP << "starting local MP game from commandline" << std::endl;

	// The setup is done equivalently to lobby MP games using as much of existing
	// code as possible.  This means that some things are set up that are not
	// needed in commandline mode, but they are required by the functions called.
	gamechat.clear_history();
	gamelist.clear();
	preferences::set_message_private(false);

	DBG_MP << "entering create mode" << std::endl;

	// Set the default parameters
	state = saved_game(); // This creates these parameters with default values defined in mp_game_settings.cpp
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
	//[era] define.
	if (cmdline_opts.multiplayer_era) {
		parameters.mp_era = *cmdline_opts.multiplayer_era;
	}
	if (const config& cfg_era = game_config.find_child("era", "id", parameters.mp_era)) {
		state.classification().era_define = cfg_era["define"].str();
	}
	else {
		std::cerr << "Could not find era '" << parameters.mp_era << "'\n";
		return;
	}
	//[multiplayer] define.
	if (cmdline_opts.multiplayer_scenario) {
		parameters.name = *cmdline_opts.multiplayer_scenario;
	}
	if (const config& cfg_multiplayer = game_config.find_child("multiplayer", "id", parameters.name)) {
		state.classification().scenario_define = cfg_multiplayer["define"].str();
	}
	else {
		std::cerr << "Could not find [multiplayer] '" << parameters.name << "'\n";
		return;
	}
	game_config_manager::get()->load_game_config_for_game(state.classification());
	state.set_carryover_sides_start(
		config_of("next_scenario", parameters.name)
	);
	state.expand_random_scenario();
	state.expand_mp_events();
	state.expand_mp_options();

	// Should number of turns be determined from scenario data?
	if (parameters.use_map_settings && state.get_starting_pos()["turns"]) {
		DBG_MP << "setting turns from scenario data: " << state.get_starting_pos()["turns"] << std::endl;
		parameters.num_turns = state.get_starting_pos()["turns"];
	}

	DBG_MP << "entering connect mode" << std::endl;

	statistics::fresh_stats();

	{
		ng::connect_engine_ptr connect_engine(new ng::connect_engine(state, true, nullptr));
		mp::connect ui(video, 0, parameters.name, game_config, gamechat, gamelist,
			*connect_engine);

		// Update the parameters to reflect game start conditions
		ui.start_game_commandline(cmdline_opts);
	}

	std::string label = "";
	if (cmdline_opts.multiplayer_label) label = *cmdline_opts.multiplayer_label;

	//resources::recorder->add_log_data("ai_log","ai_label",label);

	unsigned int repeat = (cmdline_opts.multiplayer_repeat) ? *cmdline_opts.multiplayer_repeat : 1;
	for(unsigned int i = 0; i < repeat; i++){
		saved_game state_copy(state);
		campaign_controller controller(video, state_copy, game_config, game_config_manager::get()->terrain_types());
		controller.play_game();
	}
}

void start_client(CVideo& video, const config& game_config,
	saved_game& state, const std::string& host)
{
	const config * game_config_ptr = &game_config;
	std::vector<std::string> installed_addons = ::installed_addons();
	// This function does not refer to an addon database, it calls filesystem functions.
	// For the sanity of the mp lobby, this list should be fixed for the entire lobby session,
	// even if the user changes the contents of the addon directory in the meantime.

	DBG_MP << "starting client" << std::endl;

	preferences::admin_authentication_reset r;

	gamechat.clear_history();
	gamelist.clear();
	std::unique_ptr<twesnothd_connection> wesnothd_connection = open_connection(video, host);
	if(wesnothd_connection) {
		bool re_enter;
		do {
			re_enter = false;
			try {
				enter_lobby_mode(video, *game_config_ptr, state, wesnothd_connection.get(), installed_addons);
			} catch (lobby_reload_request_exception &) {
				re_enter = true;
				game_config_manager * gcm = game_config_manager::get();
				gcm->reload_changed_game_config();
				gcm->load_game_config_for_game(state.classification()); // NOTE: Using reload_changed_game_config only doesn't seem to work here
				game_config_ptr = &gcm->game_config();

				installed_addons = ::installed_addons(); // Refersh the installed add-on list for this session.

				gamelist.clear(); //needed to make sure we update which games we have content for
				wesnothd_connection->send_data(config("refresh_lobby"));
			}
		} while (re_enter);
	}
}

mp::ui::result goto_mp_connect(CVideo& video, ng::connect_engine& engine,
	const config& game_config, twesnothd_connection* wesnothd_connection, const std::string& game_name)
{
	mp::ui::result res;

	{
		mp::connect ui(video, wesnothd_connection, game_name, game_config, gamechat, gamelist,
			engine);
		run_lobby_loop(video, ui);

		res = ui.get_result();
		if (res == mp::ui::PLAY) {
			ui.start_game();
		}
	}

	return res;
}

mp::ui::result goto_mp_wait(CVideo& video, saved_game& state, const config& game_config, twesnothd_connection* wesnothd_connection, bool observe)
{
	mp::ui::result res;

	{
		mp::wait ui(video, wesnothd_connection, game_config, state, gamechat, gamelist, false);

		ui.join_game(observe);
		run_lobby_loop(video, ui);

		res = ui.get_result();
		if (res == mp::ui::PLAY) {
			ui.start_game();
		}
	}

	return res;
}

} // end namespace mp

