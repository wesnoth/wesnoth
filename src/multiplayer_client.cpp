#include "language.hpp"
#include "multiplayer.hpp"
#include "multiplayer_client.hpp"
#include "playlevel.hpp"
#include "replay.hpp"
#include "show_dialog.hpp"

#include <sstream>

namespace {

class gamelist_manager : public gui::dialog_action
{
public:
	int do_action() {
		const network::connection res = network::receive_data(cfg);
		if(res != NULL && get_gamelist() != NULL) {
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

enum GAME_LIST_RESULT { QUIT_GAME, CREATE_GAME, JOIN_GAME };

GAME_LIST_RESULT manage_game_list(display& disp, const config* gamelist)
{
	gamelist_manager manager;
	for(;;) {
		std::vector<std::string> options;
		options.push_back(string_table["create_new_game"]);
		for(config::const_child_itors i = gamelist->child_range("game");
		    i.first != i.second; ++i.first) {
			options.push_back((**i.first)["name"]);
		}
		
		options.push_back(string_table["quit_button"]);

		const int res = gui::show_dialog(disp,NULL,"","Choose game to join",
		                       gui::MESSAGE,&options,NULL,"",NULL,&manager);
		if(res == gamelist_manager::UPDATED_GAMELIST) {
			gamelist = manager.get_gamelist();
		} else if(res == 0) {
			return CREATE_GAME;
		} else if(size_t(res) == options.size()-1) {
			return QUIT_GAME;
		} else if(res > 0 && size_t(res) < options.size()) {
			const config::const_child_itors i = gamelist->child_range("game");
			const size_t index = size_t(res)-1;
			assert(i.second - i.first > int(index));
			const std::string& id = (**(i.first+index))["id"];
			
			config response, join;
			join["id"] = id;
			response.children["join"].push_back(new config(join));
			network::send_data(response);

			return JOIN_GAME;
		}
	}

	return QUIT_GAME;
}

}

void play_multiplayer_client(display& disp, game_data& units_data, config& cfg,
                             game_state& state)
{
	const network::manager net_manager;

	std::string host;
	const int res = gui::show_dialog(disp,NULL,"","",
	                                 gui::OK_CANCEL,NULL,NULL,
	                                 string_table["remote_host"] + ": ",&host);
	if(res != 0 || host.empty()) {
		return;
	}

	network::connection sock;

	sock = network::connect(host);
	config sides, data;

	network::connection data_res = network::receive_data(data,0,5000);
	if(!data_res) {
		throw network::error(string_table["connection_timeout"]);
	}

	//if we got a gamelist back - otherwise we have
	//got a description of the game back
	const config* const gamelist = data.child("gamelist");
	if(gamelist != NULL) {
		const GAME_LIST_RESULT res = manage_game_list(disp,gamelist);
		switch(res) {
			case QUIT_GAME:
				return;
			case CREATE_GAME:
				play_multiplayer(disp,units_data,cfg,state);
				return;
			case JOIN_GAME:
				break;
		}

		for(;;) {
			data_res = network::receive_data(sides,0,5000);
			if(!data_res) {
				throw network::error(string_table["connection_timeout"]);
			}

			//if we have got valid side data
			if(sides.child("gamelist") == NULL) {
				break;
			}
		}
	} else {
		sides = data;
	}


	std::map<int,int> choice_map;
	std::vector<std::string> choices;

	std::vector<config*>& sides_list = sides.children["side"];
	for(std::vector<config*>::iterator s = sides_list.begin();
	    s != sides_list.end(); ++s) {
		if((*s)->values["controller"] == "network" &&
		   (*s)->values["taken"] != "yes") {
			choice_map[choices.size()] = 1 + s - sides_list.begin();
			choices.push_back((*s)->values["name"] + " - " +
			                  (*s)->values["type"]);
		}
	}

	const int choice = gui::show_dialog(disp,NULL,"","Choose side:",
	                                    gui::OK_CANCEL,&choices);
	if(choice < 0) {
		return;
	}

	const int team_num = choice_map[choice];

	//send our choice of team to the server
	{
		config response;
		std::stringstream stream;
		stream << team_num;
		response.values["side"] = stream.str();
		network::send_data(response);
	}

	bool got_side = false;

	for(;;) {
		config reply;
		data_res = network::receive_data(reply,0,100);
		if(data_res) {
			if(reply.values["failed"] == "yes") {
				got_side = false;
				break;
			} else if(reply.values["side_secured"].size() > 0) {
				got_side = true;
			} else if(reply.children["start_game"].empty() == false) {
				break;
			} else {
				sides = reply;
			}
		}
	}

	if(!got_side) {
		throw network::error("Choice of team unavailable.");
	}

	//we want to make the network/human players look right from our
	//perspective
	{
		std::vector<config*>& sides_list = sides.children["side"];
		for(std::vector<config*>::iterator side = sides_list.begin();
		    side != sides_list.end(); ++side) {
			string_map& values = (*side)->values;
			if(team_num-1 == side - sides_list.begin())
				values["controller"] = "human";
			else
				values["controller"] = "network";
		}
	}

	std::cerr << "starting game\n";

	state.starting_pos = sides;

	recorder.set_save_info(state);

	std::vector<config*> story;
	play_level(units_data,cfg,&sides,disp.video(),state,story);
	recorder.clear();
}
