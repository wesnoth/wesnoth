#include "language.hpp"
#include "log.hpp"
#include "multiplayer.hpp"
#include "multiplayer_client.hpp"
#include "multiplayer_lobby.hpp"
#include "playlevel.hpp"
#include "preferences.hpp"
#include "replay.hpp"
#include "scoped_resource.hpp"
#include "show_dialog.hpp"
#include "util.hpp"

#include <sstream>

namespace {

class gamelist_manager : public gui::dialog_action
{
public:
	int do_action() {
		const network::connection res = network::receive_data(cfg);
		if(res != 0 && get_gamelist() != NULL) {
			return UPDATED_GAMELIST;
		} else {
			return CONTINUE_DIALOG;
		}
	}

	const config* get_gamelist() const {
		return cfg.child("gamelist");
	}

	enum { UPDATED_GAMELIST = -3 };

private:
	config cfg;
};

void check_response(network::connection res, const config& data)
{
	if(!res) {
		throw network::error(string_table["connection_timeout"]);
	}

	const config* err = data.child("error");
	if(err != NULL) {
		throw network::error((*err)["message"]);
	}
}

void receive_gamelist(display& disp, config& data)
{
	for(;;) {

		if(data.child("gamelist")) {
			break;
		}

		const network::connection res = gui::network_data_dialog(disp,string_table["receive_game_list"],data);
		check_response(res,data);
	}
}

class wait_for_start : public lobby::dialog
{
public:
	wait_for_start(display& disp, config& cfg, int team_num, const std::string& team_name) : got_side(false), team(team_num), name(team_name), status(START_GAME), disp_(disp), cancel_button_(NULL), menu_(NULL), sides_(cfg)
	{
		SDL_Rect empty_rect = {0,0,0,0};
		area_ = empty_rect;
	}

	void generate_menu()
	{
		if(area_.h == 0) {
			return;
		}

		std::vector<std::string> details;

		const config::child_list& sides = sides_.get_children("side");
		for(config::child_list::const_iterator s = sides.begin(); s != sides.end(); ++s) {
			const config& sd = **s;

			std::string description = sd["description"];
			std::string side_name = sd["name"];
			if(s - sides.begin() == size_t(team-1)) {
				description = preferences::login();
				side_name = name;
			}

			std::stringstream str;
			str << description << "," << side_name << "," << sd["gold"] << " " << translate_string("gold") << "," << sd["team_name"];
			details.push_back(str.str());
		}

		SDL_Rect rect = area_;
		rect.h /= 2;
		SDL_FillRect(disp_.video().getSurface(),&rect,SDL_MapRGB(disp_.video().getSurface()->format,0,0,0));

		menu_.assign(new gui::menu(disp_,details,false,area_.h/2));
		menu_->set_loc(area_.x,area_.y);
		menu_->set_width(area_.w);
	}
	
	void set_area(const SDL_Rect& area) {

		area_ = area;
		generate_menu();

		const std::string text = string_table["waiting_start"];
		SDL_Rect rect = font::draw_text(NULL,disp_.screen_area(),14,font::NORMAL_COLOUR,text,0,0);
		rect.x = area.x + area.w/2 - rect.w/2;
		rect.y = area.y + (area.h*3)/4 - rect.h/2;
		font::draw_text(&disp_,rect,14,font::NORMAL_COLOUR,text,rect.x,rect.y);

		cancel_button_.assign(new gui::button(disp_,string_table["cancel"]));
		cancel_button_->set_location(area.x+area.w - cancel_button_->width() - gui::ButtonHPadding,
			                         area.y+area.h - cancel_button_->height() - gui::ButtonVPadding);
		cancel_button_->draw();
	}

	void clear_widgets() {
		cancel_button_.assign(NULL);
		menu_.assign(NULL);
	}

	lobby::RESULT process() {
		int mousex, mousey;
		const bool button = SDL_GetMouseState(&mousex,&mousey)&SDL_BUTTON_LMASK;
		if(cancel_button_->process(mousex,mousey,button)) {
			return lobby::QUIT;
		}

		if(menu_ != NULL) {
			menu_->process(mousex,mousey,button,false,false,false,false);
		}

		config reply;
		const network::connection res = network::receive_data(reply);
		if(res) {
			std::cerr << "received data while waiting: " << reply.write() << "\n";
			const config::child_list& assigns = reply.get_children("reassign_side");
			for(config::child_list::const_iterator a = assigns.begin(); a != assigns.end(); ++a) {
				if(lexical_cast_default<int>((**a)["from"]) == team) {
					team = lexical_cast_default<int>((**a)["to"]);
					generate_menu();
				}
			}

			if(reply.values["failed"] == "yes") {
				status = SIDE_UNAVAILABLE;
				return lobby::QUIT;
			} else if(reply["side_secured"].empty() == false) {
				got_side = true;
				std::cerr << "received side secured message\n";
			} else if(reply.child("start_game")) {
				std::cerr << "received start_game message\n";
				//break out of dialog
				status = START_GAME;
				return lobby::CREATE;
			} else if(reply.child("leave_game")) {
				status = GAME_CANCELLED;
				return lobby::QUIT;
			} else if(reply.child("scenario_diff")) {
				std::cerr << "received diff for scenario....applying...\n";
				sides_.apply_diff(*reply.child("scenario_diff"));
				generate_menu();
			} else if(reply.child("side")) {
				sides_ = reply;
				std::cerr << "got some sides. Current number of sides = " << sides_.get_children("side").size() << "," << reply.get_children("side").size() << "\n";
			} else {
				data_.push_back(reply);
			}
		}

		return lobby::CONTINUE;
	}

	bool got_side;
	int team;
	std::string name;
	enum { START_GAME, GAME_CANCELLED, SIDE_UNAVAILABLE } status;

private:
	display& disp_;
	config& sides_;

	util::scoped_ptr<gui::button> cancel_button_;
	util::scoped_ptr<gui::menu> menu_;
	std::deque<config> data_;

	SDL_Rect area_;

	bool manages_network() const { return true; }
	bool get_network_data(config& out) {
		if(data_.empty()) {
			return false;
		} else {
			out = data_.front();
			data_.pop_front();
			return true;
		}
	}
};
}

void play_multiplayer_client(display& disp, game_data& units_data, config& cfg,
                             game_state& state, std::string& host)
{
	log_scope("playing multiplayer client");

	const network::manager net_manager;

	if(host.empty()) {
		host = preferences::network_host();
		const int res = gui::show_dialog(disp,NULL,"","",
		                                 gui::OK_CANCEL,NULL,NULL,
		                                 string_table["remote_host"] + ": ",&host);
		if(res != 0 || host.empty()) {
			return;
		}
	}

	network::connection sock;

	const int pos = host.find_first_of(":");
 
 	if(pos == -1) {
 		sock = network::connect(host);
 	} else {
 		sock = network::connect(host.substr(0,pos),atoi(host.substr(pos+1).c_str()));
 	}
 
	config sides, data;

	network::connection data_res = gui::network_data_dialog(disp,string_table["connecting_remote"],data);
	check_response(data_res,data);

	preferences::set_network_host(host);

	//if response contained a version number
	const std::string& version = data["version"];
	if(version.empty() == false && version != game_config::version) {
		throw network::error("The server requires version '" + version
		      + "' while you are using version '" + game_config::version + "'");
	}

	bool logged_in = false;

	//if we got a direction to login
	if(data.child("mustlogin")) {

		bool first_time = true;
		config* error = NULL;

		do {
			if(error != NULL) {
				gui::show_dialog(disp,NULL,"",(*error)["message"],gui::OK_ONLY);
			}

			std::string login = preferences::login();

			if(!first_time) {	
				const int res = gui::show_dialog(disp,NULL,"",
				                    string_table["must_login"],gui::OK_CANCEL,
									NULL,NULL,string_table["login"] + ": ",&login);
				if(res != 0 || login.empty()) {
					return;
				}

				preferences::set_login(login);
			}

			first_time = false;

			config response;
			response.add_child("login")["username"] = login;
			network::send_data(response);
	
			data_res = network::receive_data(data,0,3000);
			if(!data_res) {
				throw network::error(string_table["connection_timeout"]);
			}

			std::cerr << "login response: '" << data.write() << "'\n";

			error = data.child("error");
		} while(error != NULL);

		logged_in = true;
	}

	for(bool first_time = true; (first_time || logged_in) && network::nconnections() > 0;
	    first_time = false) {

		if(!first_time) {
			receive_gamelist(disp,data);
		}

		std::cerr << "when receiving gamelist got '" << data.write() << "'\n";

		bool observer = false;

		//if we got a gamelist back - otherwise we have
		//got a description of the game back
		const config* const gamelist = data.child("gamelist");
		if(gamelist != NULL) {
			config game_data = data;
			int status = -1;

			std::vector<std::string> chat_messages;
			while(status == -1) {
				const lobby::RESULT res = lobby::enter(disp,game_data,cfg,NULL,chat_messages);
				switch(res) {
					case lobby::QUIT: {
						status = 1;
						return;
					}
					case lobby::CREATE: {
						multiplayer_game_setup_dialog mp_dialog(disp,units_data,cfg,state,false);
						lobby::RESULT res = lobby::CONTINUE;
						while(res == lobby::CONTINUE) {
							res = lobby::enter(disp,game_data,cfg,&mp_dialog,chat_messages);
						}

						if(res == lobby::CREATE) {
							mp_dialog.start_game();
						}

						status = -1;

						break;
					}

					case lobby::OBSERVE:
						observer = true;
					case lobby::JOIN: {
						status = 1;
						break;
					}
				}
			}
    
			for(;;) {
				data_res = gui::network_data_dialog(disp,string_table["getting_game_data"],sides);
				if(data_res && sides.child("error")) {
					gui::show_dialog(disp,NULL,"",(*sides.child("error"))["message"]);
					break;
				}

				check_response(data_res,sides);
    
				//if we have got valid side data
				if(sides.child("gamelist") == NULL) {
					break;
				}
			}

			if(sides.child("error")) {
				continue;
			}
		} else {
			sides = data;
		}

		//ensure we send a close game message to the server when we are done
		network_game_manager game_manager;

		const config::child_list& sides_list = sides.get_children("side");

		int team_num = 0;
		std::string team_name;
		
		if(!observer) {
			//search for an appropriate vacant slot. If a description is set
			//(i.e. we're loading from a saved game), then prefer to get the side
			//with the same description as our login. Otherwise just choose the first
			//available side.
			config::child_list::const_iterator side_choice = sides_list.end();
			int nchoice = -1, n = 1;
			bool allow_changes = false;
			std::string default_race;
			for(config::child_list::const_iterator s = sides_list.begin(); s != sides_list.end(); ++s, ++n) {
				if((**s)["controller"] == "network" &&
				   (**s)["taken"] != "yes") {
					if(side_choice == sides_list.end() || (**s)["description"] == preferences::login()) {
						side_choice = s;
						nchoice = n;
						allow_changes = (**s)["allow_changes"] != "no";
						default_race = (**s)["name"];
					}
				}
			}

			if(side_choice == sides_list.end()) {
				gui::show_dialog(disp,NULL,"",string_table["no_sides_available"],gui::OK_ONLY);
				continue;
			}

			team_num = nchoice;

			config response;
			response["side"] = lexical_cast<std::string>(nchoice);
			response["description"] = preferences::login();

			const std::string& era = sides["era"];

			const config* const era_cfg = cfg.find_child("era","id",era);

			if(era_cfg == NULL) {
				std::cerr << "era '" << era << "' not found\n";
				return;
			}

			const config::child_list& possible_sides = era_cfg->get_children("multiplayer_side");

			if(possible_sides.empty()) {
				std::cerr << "no multiplayer sides found\n";
				return;
			}

			size_t choice = 0;

			std::vector<std::string> choices;
			for(config::child_list::const_iterator side =
			    possible_sides.begin(); side != possible_sides.end(); ++side) {
				choices.push_back(translate_string_default((**side)["id"],(**side)["name"]));

				if(choices.back() == default_race) {
					choice = side - possible_sides.begin();
				}
			}

			//if the client is allowed to choose their team, instead of having
			//it set by the server, do that here.
			if(allow_changes) {
				choice = size_t(gui::show_dialog(disp,NULL,"",
				     string_table["client_choose_side"],gui::OK_ONLY,&choices));
			}

			assert(choice < possible_sides.size());

			const config& chosen_side = *possible_sides[choice];
			team_name = response["name"] = chosen_side["name"];
			response["type"] = chosen_side["type"];
			response["recruit"] = chosen_side["recruit"];
			response["music"] = chosen_side["music"];

			network::send_data(response);
		}
    
		wait_for_start waiter(disp,sides,team_num,team_name);
		std::vector<std::string> messages;
		config game_data;
		lobby::RESULT dialog_res = lobby::CONTINUE;
		while(dialog_res == lobby::CONTINUE) {
			dialog_res = lobby::enter(disp,game_data,cfg,&waiter,messages);
		}

		waiter.clear_widgets();

		if(waiter.status == wait_for_start::GAME_CANCELLED) {
			gui::show_dialog(disp,NULL,"",string_table["game_cancelled"],
			                 gui::OK_ONLY);
			continue;
		} else if(waiter.status == wait_for_start::SIDE_UNAVAILABLE) {
			gui::show_dialog(disp,NULL,"",string_table["side_unavailable"],
			                 gui::OK_ONLY);
			continue;			
		} else if(dialog_res != lobby::CREATE) {
			continue;
		}
    
		if(!observer && !waiter.got_side) {
			throw network::error("Choice of team unavailable.");
		}

		team_num = waiter.team;
    
		//we want to make the network/human players look right from our
		//perspective
		{
			const config::child_list& sides_list = sides.get_children("side");
			for(config::child_list::const_iterator side = sides_list.begin(); side != sides_list.end(); ++side) {
				if(team_num-1 == side - sides_list.begin()) {
					(**side)["controller"] = preferences::client_type();
				} else if((**side)["controller"] != "null") {
					(**side)["controller"] = "network";
				}
			}
		}
    
		//any replay data is only temporary and should be removed from
		//the level data in case we want to save the game later
		config* const replay_data = sides.child("replay");
		config replay_data_store;
		if(replay_data != NULL) {
			replay_data_store = *replay_data;
			std::cerr << "setting replay\n";
			recorder = replay(replay_data_store);
			if(!recorder.empty()) {
				const int res = gui::show_dialog(disp,NULL,
		               "", string_table["replay_game_message"],
					   gui::YES_NO);
				//if yes, then show the replay, otherwise
				//skip showing the replay
				if(res == 0) {
					sides = state.starting_pos;
					recorder.set_skip(0);
				} else {
					std::cerr << "skipping...\n";
					recorder.set_skip(-1);
				}
			}
    
			sides.clear_children("replay");
		}
    
		std::cerr << "starting game\n";

		state.starting_pos = sides;
		state.snapshot = sides;
		state.can_recruit.clear();
    
		recorder.set_save_info(state);
    
		std::vector<config*> story;
		play_level(units_data,cfg,&sides,disp.video(),state,story);
		recorder.clear();
	}
}
