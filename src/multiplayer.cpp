/* $Id$ */
/*
   Copyright (C) 2007 - 2008
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
#include "game_config.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "multiplayer.hpp"
#include "multiplayer_ui.hpp"
#include "multiplayer_connect.hpp"
#include "multiplayer_wait.hpp"
#include "multiplayer_lobby.hpp"
#include "multiplayer_create.hpp"
#include "playmp_controller.hpp"
#include "network.hpp"
#include "playcampaign.hpp"
#include "game_preferences.hpp"
#include "preferences_display.hpp"
#include "random.hpp"
#include "replay.hpp"
#include "video.hpp"
#include "statistics.hpp"
#include "serialization/string_utils.hpp"
#include "upload_log.hpp"
#include "wml_separators.hpp"

#define LOG_NW LOG_STREAM(info, network)

namespace {

class network_game_manager
{
public:
	// Add a constructor to avoid stupid warnings with some versions of GCC
	network_game_manager() {};

	~network_game_manager()
	{
		if(network::nconnections() > 0) {
			LOG_NW << "sending leave_game\n";
			config cfg;
			cfg.add_child("leave_game");
			network::send_data(cfg, 0, true);
			LOG_NW << "sent leave_game\n";
		}
	};
};

}

static void run_lobby_loop(display& disp, mp::ui& ui)
{
	disp.video().modeChanged();
	bool first = true;
	font::cache_mode(font::CACHE_LOBBY);
	while (ui.get_result() == mp::ui::CONTINUE) {
		if (disp.video().modeChanged() || first) {
			SDL_Rect lobby_pos = { 0, 0, disp.video().getx(), disp.video().gety() };
			ui.set_location(lobby_pos);
			first = false;
		}

		events::raise_process_event();
		events::raise_draw_event();
		events::pump();

		ui.process_network();

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

class server_button : public gui::dialog_button
{
public:
	server_button(CVideo &vid): dialog_button(vid, _("View List"))
	{}
protected:
	int action(gui::dialog_process_info &dp_info)
	{
		//display a dialog with a list of known servers
		gui::dialog server_dialog(dialog()->get_display(), _("List of Servers"),
			_("Choose a known server from the list"), gui::OK_CANCEL);
		std::vector<std::string> servers;
		std::ostringstream menu_heading;
		menu_heading << HEADING_PREFIX << _("Name") << COLUMN_SEPARATOR << _("Address");
		servers.push_back(menu_heading.str());
		const std::vector<game_config::server_info>& pref_servers = preferences::server_list();
		std::vector<game_config::server_info>::const_iterator server;
		for(server = pref_servers.begin(); server != pref_servers.end(); ++server) {
			servers.push_back(server->name + COLUMN_SEPARATOR + server->address);
		}
		server_dialog.set_menu(servers);
		gui::menu::basic_sorter server_sorter;
		server_sorter.set_alpha_sort(0).set_id_sort(1);
		server_dialog.get_menu().set_sorter(&server_sorter);
		if(server_dialog.show() >= 0) {
			//now save the result back to the parent dialog
			dialog()->get_textbox().set_text(preferences::server_list()[server_dialog.result()].address);
		}
		//the button state should be cleared after popping up an intermediate dialog
		dp_info.clear_buttons();
		return gui::CONTINUE_DIALOG;
	}
};
}

static server_type open_connection(game_display& disp, const std::string& original_host)
{
	std::string h = original_host;

	if(h.empty()) {
		gui::dialog d(disp, _("Connect to Host"), "", gui::OK_CANCEL);
		d.set_textbox(_("Choose host to connect to: "), preferences::network_host());
		d.add_button( new server_button(disp.video()), gui::dialog::BUTTON_EXTRA);
		if(d.show() || d.textbox_text().empty()) {
			return ABORT_SERVER;
		}
		h = d.textbox_text();
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

	config::child_list redirects;
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

		// Backwards-compatibility "version" attribute
		const std::string& version = data["version"];
		if(version.empty() == false && version != game_config::version) {
			utils::string_map i18n_symbols;
			i18n_symbols["version1"] = version;
			i18n_symbols["version2"] = game_config::version;
			const std::string errorstring = vgettext("The server requires version '$version1' while you are using version '$version2'", i18n_symbols);
			throw network::error(errorstring);
		}

		// Check for "redirect" messages
		if(data.child("redirect")) {
			config* redirect = data.child("redirect");

			host = (*redirect)["host"];
			port = lexical_cast_default<unsigned int>((*redirect)["port"], 15000);

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
			network::send_data(res, 0, true);
		}

		//if we got a direction to login
		if(data.child("mustlogin")) {
			bool first_time = true;
			config* error = NULL;

            std::vector<std::string> opts;
            opts.push_back(_("Log in with password"));
            opts.push_back(_("Request password reminder for this username"));
            opts.push_back(_("Choose a different username"));

			do {
                std::string login = preferences::login();
				std::string password = "";
				std::string password_reminder = "";

				if(!first_time) {

				    //Somewhat hacky implementation

				    //! @todo A fancy textbox that displays characters as dots or asterisk would
				    //! be nice, just in chase your enemy is standing behind you

				    if((*error).child("password_request")) {
                        const int res = gui::show_dialog(disp, NULL, _("Login"),
                                (*error)["message"], gui::OK_CANCEL,
                                &opts, NULL, _("Password: "), &password, mp::max_login_size);

                        switch(res) {
                            //Log in with password
                            case 0:
                                break;
                            //Request a password reminder
                            case 1:
                                password_reminder = login;
                                break;
                            //Choose a different username
                            case 2:
                                password = "";
                                goto new_username;
                                break;
                            default: return ABORT_SERVER;
                        }

				    } else {
                        if(error != NULL) {
                            gui::dialog(disp,"",(*error)["message"],gui::OK_ONLY).show();
                        }

				        new_username:

                        const int res = gui::show_dialog(disp, NULL, "",
                                _("You must log in to this server"), gui::OK_CANCEL,
                                NULL, NULL, _("Login: "), &login, mp::max_login_size);
                        if(res != 0 || login.empty()) {
                            return ABORT_SERVER;
                        }
                        preferences::set_login(login);
				    }
				}

				first_time = false;

				config response;
				response.add_child("login")["username"] = login;
				(*(response.child("login")))["password"] = password;
				(*(response.child("login")))["password_reminder"] = password_reminder;

				network::send_data(response, 0, true);

				network::connection data_res = network::receive_data(data, 0, 3000);
				if(!data_res) {
					throw network::error(_("Connection timed out"));
				}

				error = data.child("error");
			} while(error != NULL);
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

static void enter_wait_mode(game_display& disp, const config& game_config, game_data& data, mp::chat& chat, config& gamelist, bool observe)
{
	mp::ui::result res;
	game_state state;
	network_game_manager m;
	upload_log nolog(false);

	gamelist.clear();
	statistics::fresh_stats();

	{
		mp::wait ui(disp, game_config, data, chat, gamelist);

		ui.join_game(observe);

		run_lobby_loop(disp, ui);
		res = ui.get_result();

		if (res == mp::ui::PLAY) {
			ui.start_game();
			// FIXME commented a pointeless if since the else does exactly the same thing
			//if (preferences::skip_mp_replay()){
				//FIXME implement true skip replay
				//state = ui.request_snapshot();
				//state = ui.get_state();
			//}
			//else{
				state = ui.get_state();
			//}
		}
	}

	switch (res) {
	case mp::ui::PLAY:
		play_game(disp, state, game_config, data, nolog, IO_CLIENT,
			preferences::skip_mp_replay() && observe);
		recorder.clear();

		break;
	case mp::ui::QUIT:
	default:
		break;
	}
}

static void enter_create_mode(game_display& disp, const config& game_config, game_data& data, mp::chat& chat, config& gamelist, mp::controller default_controller, bool is_server);

static void enter_connect_mode(game_display& disp, const config& game_config, game_data& data,
		mp::chat& chat, config& gamelist, const mp::create::parameters& params,
		mp::controller default_controller, bool is_server)
{
	mp::ui::result res;
	game_state state;
	const network::manager net_manager(1,1);
	const network::server_manager serv_manager(15000, is_server ?
			network::server_manager::TRY_CREATE_SERVER :
			network::server_manager::NO_SERVER);
	network_game_manager m;
	upload_log nolog(false);

	gamelist.clear();
	statistics::fresh_stats();

	{
		mp::connect ui(disp, game_config, data, chat, gamelist, params, default_controller);
		run_lobby_loop(disp, ui);

		res = ui.get_result();

		// start_game() updates the parameters to reflect game start,
		// so it must be called before get_level()
		if (res == mp::ui::PLAY) {
			ui.start_game();
			state = ui.get_state();
		}
	}

	switch (res) {
	case mp::ui::PLAY:
		play_game(disp, state, game_config, data, nolog, IO_SERVER);
		recorder.clear();

		break;
	case mp::ui::CREATE:
		enter_create_mode(disp, game_config, data, chat, gamelist, default_controller, is_server);
		break;
	case mp::ui::QUIT:
	default:
		network::send_data(config("refresh_lobby"), 0, true);
		break;
	}
}

static void enter_create_mode(game_display& disp, const config& game_config, game_data& data, mp::chat& chat, config& gamelist, mp::controller default_controller, bool is_server)
{
	mp::ui::result res;
	mp::create::parameters params;

	{
		mp::create ui(disp, game_config, chat, gamelist);
		run_lobby_loop(disp, ui);
		res = ui.get_result();
		params = ui.get_parameters();
	}

	switch (res) {
	case mp::ui::CREATE:
		enter_connect_mode(disp, game_config, data, chat, gamelist, params, default_controller, is_server);
		break;
	case mp::ui::QUIT:
	default:
		//update lobby content
		network::send_data(config("refresh_lobby"), 0, true);
		break;
	}
}

static void enter_lobby_mode(game_display& disp, const config& game_config, game_data& data, mp::chat& chat, config& gamelist)
{
	mp::ui::result res;

	while (true) {
		{
			mp::lobby ui(disp, game_config, chat, gamelist);
			run_lobby_loop(disp, ui);
			res = ui.get_result();
		}

		switch (res) {
		case mp::ui::JOIN:
			try {
				enter_wait_mode(disp, game_config, data, chat, gamelist, false);
			} catch(config::error& error) {
				if(!error.message.empty()) {
					gui::show_error_message(disp, error.message);
				}
				//update lobby content
				network::send_data(config("refresh_lobby"), 0, true);
			}
			break;
		case mp::ui::OBSERVE:
			try {
				enter_wait_mode(disp, game_config, data, chat, gamelist, true);
			} catch(config::error& error) {
				if(!error.message.empty()) {
					gui::show_error_message(disp, error.message);
				}
			}
			//update lobby content
			network::send_data(config("refresh_lobby"), 0, true);
			break;
		case mp::ui::CREATE:
			try {
				enter_create_mode(disp, game_config, data, chat, gamelist, mp::CNTR_NETWORK, false);
			} catch(config::error& error) {
				if (!error.message.empty())
					gui::show_error_message(disp, error.message);
				//update lobby content
				network::send_data(config("refresh_lobby"), 0, true);
			}
			break;
		case mp::ui::QUIT:
			return;
		case mp::ui::PREFERENCES:
			{
				const preferences::display_manager disp_manager(&disp);
				preferences::show_preferences_dialog(disp,game_config);
				//update lobby content
				network::send_data(config("refresh_lobby"), 0, true);
			}
			break;
		default:
			return;
		}
	}
}

namespace mp {

void start_server(game_display& disp, const config& game_config, game_data& data,
		mp::controller default_controller, bool is_server)
{
	const set_random_generator generator_setter(&recorder);
	mp::chat chat;
	config gamelist;
	playmp_controller::set_replay_last_turn(0);
	preferences::set_message_private(false);
	enter_create_mode(disp, game_config, data, chat, gamelist, default_controller, is_server);
}

void start_client(game_display& disp, const config& game_config, game_data& data,
		const std::string host)
{
	const set_random_generator generator_setter(&recorder);
	const network::manager net_manager(1,1);

	mp::chat chat;
	config gamelist;
	server_type type = open_connection(disp, host);

	switch(type) {
	case WESNOTHD_SERVER:
		enter_lobby_mode(disp, game_config, data, chat, gamelist);
		break;
	case SIMPLE_SERVER:
		playmp_controller::set_replay_last_turn(0);
		preferences::set_message_private(false);
		enter_wait_mode(disp, game_config, data, chat, gamelist, false);
		break;
	case ABORT_SERVER:
		break;
	}
}

}

