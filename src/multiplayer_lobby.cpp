#include "global.hpp"

#include "events.hpp"
#include "filesystem.hpp"
#include "font.hpp"
#include "game_config.hpp"
#include "hotkeys.hpp"
#include "image.hpp"
#include "key.hpp"
#include "language.hpp"
#include "multiplayer_lobby.hpp"
#include "network.hpp"
#include "preferences.hpp"
#include "show_dialog.hpp"
#include "sound.hpp"
#include "statistics.hpp"
#include "widgets/textbox.hpp"
#include "widgets/button.hpp"
#include "widgets/menu.hpp"

#include "SDL.h"

#include <sstream>

namespace {
	int xscale(display& disp, int x)
	{
		return (x*disp.x())/1024;
	}

	int yscale(display& disp, int y)
	{
		return (y*disp.y())/768;
	}
}

namespace lobby {

RESULT enter(display& disp, config& game_data, const config& terrain_data, dialog* dlg,
			 std::vector<std::string>& messages)
{
	statistics::fresh_stats();

	std::cerr << "entered multiplayer lobby...\n";
	const preferences::display_manager disp_manager(&disp);
	const hotkey::basic_handler key_handler(&disp);
	const tooltips::manager tooltips_manager(disp);
	disp.video().modeChanged(); // resets modeChanged value

	CKey key;

	surface background(image::get_image("misc/lobby.png",image::UNSCALED));
	background.assign(scale_surface(background,disp.x(),disp.y()));

	if(background == NULL) {
		return QUIT;
	}

	SDL_BlitSurface(background, NULL, disp.video().getSurface(), NULL);
	update_whole_screen();

	gui::textbox message_entry(disp,500);

	bool last_escape = true;

	int game_selection = 0, user_selection = 0;

	for(bool first_time = true; ; first_time = false) {
		message_entry.set_focus(true);

		const SDL_Rect dlg_rect = {xscale(disp,12),yscale(disp,42),xscale(disp,832),yscale(disp,518)};

		//if the dialog is present, back it up before we repaint the entire screen
		surface_restorer dlg_restorer;
		if(dlg != NULL && first_time == false) {
			dlg_restorer = surface_restorer(&disp.video(),dlg_rect);
		}

		SDL_BlitSurface(background, NULL, disp.video().getSurface(), NULL);

		dlg_restorer.restore();
		dlg_restorer = surface_restorer();

		const SDL_Rect chat_area = { xscale(disp,12), yscale(disp,576), xscale(disp,832), yscale(disp,142) };

		gui::textbox chat_textbox(disp,chat_area.w,"",false);

		std::vector<std::string> options;
		std::vector<bool> game_vacant_slots, game_observers;

		const config* const gamelist = game_data.child("gamelist");

		if(dlg == NULL) {
			// Game List GUI
			if(gamelist == NULL) {
				std::cerr << "ERROR: could not find gamelist\n";
				return QUIT;
			}
	
			config::const_child_itors i;
			for(i = gamelist->child_range("game"); i.first != i.second; ++i.first) {
	
				std::cerr << "game data here:" << (**i.first).write() << "end game data here\n";
	
				std::stringstream str;
	
				//if this is the item that should be selected, make it selected by default
				if(game_selection-- == 0) {
					str << "*";
				}
	
				std::string map_data = (**i.first)["map_data"];
				if(map_data == "") {
					map_data = read_map((**i.first)["map"]);
				}
	
				if(map_data != "") {
					try {
						gamemap map(terrain_data,map_data);
						const surface mini(image::getMinimap(100,100,map,0));
	
						//generate a unique id to show the map as
						char buf[50];
						sprintf(buf,"addr %p", (SDL_Surface*)mini);
	
						image::register_image(buf,mini);
	
						str << "&" << buf << ",";
					} catch(gamemap::incorrect_format_exception& e) {
						std::cerr << "illegal map: " << e.msg_ << "\n";
					}
				} else {
					str << "(" << _("Shroud") << "),";
				}
	
				std::string name = (**i.first)["name"];
				if(name.size() > 30)
					name.resize(30);
	
				str << name;
	
				const std::string& turn = (**i.first)["turn"];
				const std::string& slots = (**i.first)["slots"];
				if(turn != "") {
					str << "," << _("Turn") << " " << turn;
				} else if(slots != "") {
					str << "," << slots << " " << gettext(slots == "1" ? N_("Vacant Slot") : N_("Vacant Slots"));
				}
	
				options.push_back(str.str());
				game_vacant_slots.push_back(slots != "" && slots != "0");
				game_observers.push_back((**i.first)["observer"] != "no");
			}
		}

		const bool games_available = dlg == NULL && options.empty() == false;
		if(!games_available) {
			options.push_back("<no games open>");
		}

		gui::menu games_menu(disp,options);
		gui::button observe_game(disp,_("Observe Game"));
		gui::button join_game(disp,_("Join Game"));
		gui::button new_game(disp,_("Create Game"));
		gui::button quit_game(disp,_("Quit"));

		if(dlg != NULL) {
			observe_game.hide();
			join_game.hide();
			new_game.hide();
			quit_game.hide();
		}

		if(games_menu.selection() >= 0 && games_menu.selection() < int(game_vacant_slots.size())) {
			assert(game_vacant_slots.size() == game_observers.size());

			join_game.hide(!game_vacant_slots[games_menu.selection()]);
			observe_game.hide(!game_observers[games_menu.selection()]);
		} else {
			observe_game.hide();
			join_game.hide();
		}
		
		std::vector<std::string> users;
		for(config::const_child_itors i = game_data.child_range("user"); i.first != i.second; ++i.first) {
			std::string name = (**i.first)["name"];
			if(name.size() > 30)
				name.resize(30);

			const std::string avail = (**i.first)["available"];

			//display unavailable players in red
			if(avail == "no") {
				name.insert(name.begin(),'#');
			}

			//if this user should be selected
			if(user_selection-- == 0) {
				name.insert(name.begin(),'*');
			}

			users.push_back(name);
		}

		if(users.empty()) {
			users.push_back(preferences::login());
		}

		std::cerr << "have " << users.size() << " users\n";

		if(users.empty()) {
			std::cerr << "ERROR: empty user list received\n";
			users.push_back("error");
		}

		gui::menu users_menu(disp,users);

		// Set GUI locations
		users_menu.set_width(xscale(disp,156));
		users_menu.set_location(xscale(disp,856),yscale(disp,42));

		update_rect(xscale(disp,856),yscale(disp,42),xscale(disp,156),yscale(disp,708));

		if(dlg != NULL) {
			if(first_time) {
				dlg->set_area(dlg_rect);
			}
		} else {
			games_menu.set_width(xscale(disp,832));
			games_menu.set_location(xscale(disp,12),yscale(disp,42));
		}

		update_rect(xscale(disp,12),yscale(disp,42),xscale(disp,832),yscale(disp,518));
		join_game.set_location(xscale(disp,12),yscale(disp,7));
		observe_game.set_location(join_game.location().x + join_game.location().w + 5,yscale(disp,7));
		new_game.set_location(observe_game.location().x + observe_game.location().w + 5,yscale(disp,7));
		quit_game.set_location(new_game.location().x + new_game.location().w + 5,yscale(disp,7));
		message_entry.set_location(xscale(disp,14),yscale(disp,732));
		message_entry.set_width(xscale(disp,830));

		update_whole_screen();

		bool old_enter = true;

		size_t last_message = messages.size() < 50 ? 0 : messages.size() - 50;

		for(;;) {

			if(last_message < messages.size()) {
				// Display Chats
				std::stringstream text;
				for(; last_message != messages.size(); ++last_message) {
					text << messages[last_message];
					if(last_message+1 != messages.size()) {
						text << "\n";
					}
				}

				chat_textbox.append_text(text.str());

				chat_textbox.set_location(chat_area);
				chat_textbox.set_wrap(true);
				chat_textbox.scroll_to_bottom();
				chat_textbox.set_dirty();
			}

			int mousex, mousey;
			SDL_GetMouseState(&mousex,&mousey);
			tooltips::process(mousex, mousey);

			if(dlg != NULL) {
				const RESULT res = dlg->process();
				if(res != CONTINUE) {
					return res;
				}
			} else {
				games_menu.process();

				if(games_menu.selection() >= 0 && games_menu.selection() < int(game_vacant_slots.size())) {
					join_game.hide(!game_vacant_slots[games_menu.selection()]);
					observe_game.hide(!game_observers[games_menu.selection()]);
				}
			}

			users_menu.process();
			 
			const bool double_click = games_menu.double_clicked();
			const bool observe = observe_game.pressed() || !games_available && double_click;
			if(games_available && (observe || join_game.pressed() || double_click)) {
				const size_t index = size_t(games_menu.selection());
				const config::const_child_itors i = gamelist->child_range("game");
				assert(index < size_t(i.second - i.first));
				const std::string& id = (**(i.first+index))["id"];

				config response;
				config& join = response.add_child("join");
				join["id"] = id;
				network::send_data(response);
				return observe ? OBSERVE : JOIN;
			}
			
			if(dlg == NULL && new_game.pressed()) {
				return CREATE;
				break;
			}

			const bool enter = key[SDLK_RETURN] && !old_enter;
			old_enter = key[SDLK_RETURN] != 0;
			if(enter && message_entry.text().empty() == false) {
				const std::string& text = message_entry.text();

				static const std::string query = "/query ";
				if(text.size() >= query.size() && std::equal(query.begin(),query.end(),text.begin())) {
					const std::string args = text.substr(query.size());

					config cfg;
					cfg.add_child("query")["type"] = args;
					network::send_data(cfg);
				} else {

					config msg;
					config& child = msg.add_child("message");
					child["message"] = text;
					child["sender"] = preferences::login();
					network::send_data(msg);

					std::stringstream message;
					message << "<" << child["sender"] << ">  " << child["message"];
					messages.push_back(message.str());
				}

				message_entry.clear();
			}

			if(last_escape == false && key[SDLK_ESCAPE] || dlg == NULL && quit_game.pressed()){
				return QUIT;
			}

			last_escape = key[SDLK_ESCAPE] != 0;

			events::raise_process_event();
			events::raise_draw_event();

			chat_textbox.process();

			user_selection = users_menu.selection();
			game_selection = games_menu.selection();

			//if the list is refreshed, we want to redraw the entire screen
			config data;
			bool got_data = false;
			if(dlg == NULL || dlg->manages_network() == false) {
				const network::connection res = network::receive_data(data);
				if(res) {
					got_data = true;
				}
			} else if(dlg != NULL && dlg->manages_network()) {
				got_data = dlg->get_network_data(data);
			}

			if(got_data) {
				if(data.child("gamelist")) {
					game_data = data;
					break;
				} else if(data.child("gamelist_diff")) {
					const int old_users = game_data.get_children("user").size();
					game_data.apply_diff(*data.child("gamelist_diff"));
					const int new_users = game_data.get_children("user").size();
					if(new_users < old_users) {
						sound::play_sound(game_config::sounds::user_leave);
					} else if(new_users > old_users) {
						sound::play_sound(game_config::sounds::user_arrive);
					}

					break;
				} else if(data.child("error")) {
					throw network::error((*data.child("error"))["message"]);
				} else if(data.child("message")) {
					sound::play_sound(game_config::sounds::receive_message);

					const config& msg = *data.child("message");
					std::stringstream message;
					message << "<" << msg["sender"] << ">  " << msg["message"];
					messages.push_back(message.str());
				}
			}

			if(disp.video().modeChanged()) {
				if (dlg != NULL)
					dlg->clear_area();
				return CONTINUE;
			}

			events::pump();
			disp.video().flip();
			SDL_Delay(20);
		}
	}
}

}
