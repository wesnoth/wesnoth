#include "events.hpp"
#include "font.hpp"
#include "key.hpp"
#include "language.hpp"
#include "multiplayer_lobby.hpp"
#include "network.hpp"
#include "show_dialog.hpp"
#include "textbox.hpp"
#include "widgets/button.hpp"
#include "widgets/menu.hpp"

#include "SDL.h"

#include <sstream>

namespace lobby {

RESULT enter(display& disp, config& game_data)
{
	CKey key;

	const int border = 5;

	std::vector<std::string> messages;

	for(;;) {
		std::cerr << "game data: " << game_data.write() << "\n";
		gui::draw_solid_tinted_rectangle(0,0,disp.x()-1,disp.y()-1,0,0,0,1.0,
		                    disp.video().getSurface());

		std::stringstream text;
		for(size_t n = messages.size() > 8 ? messages.size()-8 : 0;
		    n != messages.size(); ++n) {
			text << messages[n] << "\n";
		}

		gui::textbox message_entry(disp,300);
		message_entry.set_location(border,
		                           disp.y()-border-message_entry.height());

		gui::button send_message(disp,"Send Message");
		send_message.set_xy(border*2+message_entry.width(),
		                    disp.y()-border-send_message.height());

		font::draw_text(&disp,disp.screen_area(),12,font::NORMAL_COLOUR,
		                text.str(),border,400);

		const config* const gamelist = game_data.child("gamelist");
		assert(gamelist != NULL);

		std::vector<std::string> options;
		config::const_child_itors i;
		for(i = gamelist->child_range("game"); i.first != i.second; ++i.first) {
			std::string name = (**i.first)["name"];
			if(name.size() > 30)
				name.resize(30);
			options.push_back(name);
		}

		const bool games_available = options.empty() == false;
		if(!games_available) {
			options.push_back("<no games open>");
		}

		gui::menu games_menu(disp,options);
		games_menu.set_loc(border,border);
		gui::button join_game(disp,string_table["join_game"]);
		gui::button new_game(disp,string_table["create_new_game"]);
		gui::button quit_game(disp,string_table["quit_button"]);
		
		const int buttons_y = border*2 + games_menu.height();
		join_game.set_xy(border,buttons_y);
		new_game.set_xy(border*2+join_game.width(),buttons_y);
		quit_game.set_xy(border*3+join_game.width()+new_game.width(),buttons_y);

		std::vector<std::string> users;
		for(i = game_data.child_range("user"); i.first != i.second; ++i.first) {
			std::string name = (**i.first)["name"];
			if(name.size() > 30)
				name.resize(30);
			users.push_back(name);
		}

		assert(users.empty() == false);

		gui::menu users_menu(disp,users);
		users_menu.set_loc(disp.x()-border-users_menu.width(),border);

		for(;;) {
			int mousex, mousey;
			const int mouse_flags = SDL_GetMouseState(&mousex,&mousey);

			const bool left_button = mouse_flags&SDL_BUTTON_LMASK;

			games_menu.process(mousex,mousey,left_button,
			                   key[SDLK_UP],key[SDLK_DOWN],
			                   key[SDLK_PAGEUP],key[SDLK_PAGEDOWN]);

			users_menu.process(mousex,mousey,left_button,
			                   key[SDLK_UP],key[SDLK_DOWN],
			                   key[SDLK_PAGEUP],key[SDLK_PAGEDOWN]);
			 
			if(games_available &&
			   join_game.process(mousex,mousey,left_button)) {
				const size_t index = size_t(games_menu.selection());
				const config::const_child_itors i
				                   = gamelist->child_range("game");
				assert(index < size_t(i.second - i.first));
				const std::string& id = (**(i.first+index))["id"];

				config response;
				config& join = response.add_child("join");
				join["id"] = id;
				network::send_data(response);
				return JOIN;
			}
			
			if(new_game.process(mousex,mousey,left_button)) {
				std::string name;
				const int res = gui::show_dialog(disp,NULL,"","Name your game:",
				                   gui::OK_CANCEL,NULL,NULL,"Name:",&name);
				if(res == 0 && !name.empty()) {
					config response;
					config create_game;
					create_game["name"] = name;
					response.children["create_game"].push_back(
					                                new config(create_game));
					network::send_data(response);
	
					return CREATE;
				}

				break;
			}

			if(send_message.process(mousex,mousey,left_button)) {
				config msg;
				config& child = msg.add_child("message");
				child["message"] = message_entry.text();
				network::send_data(msg);
				message_entry.clear();
			}

			if(quit_game.process(mousex,mousey,left_button)) {
				return QUIT;
			}

			message_entry.process();

			config data;

			//if the list is refreshed, we want to redraw the entire screen
			const network::connection res = network::receive_data(data);
			if(res) {
				if(data.child("gamelist")) {
					game_data = data;
					break;
				} else if(data.child("error")) {
					throw network::error((*data.child("error"))["message"]);
				} else if(data.child("message")) {
					const config& msg = *data.child("message");
					std::stringstream message;
					message << "<" << msg["sender"] << ">  " << msg["message"];
					messages.push_back(message.str());
					break;
				}
			}

			events::pump();
			SDL_Delay(20);
		}
	}
}

}
