#include "global.hpp"

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
#include "wassert.hpp"

#include <sstream>

#define LOG_NW lg::info(lg::network)
#define ERR_NW lg::err(lg::network)

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

	~gamelist_manager() {} // Workaround for IRIX Mips CC

	enum { UPDATED_GAMELIST = -3 };

private:
	config cfg;
};

void check_response(network::connection res, const config& data)
{
	if(!res) {
		throw network::error(_("Connection timed out"));
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

		const network::connection res = gui::network_data_dialog(disp,_("Receiving game list..."),data);
		check_response(res,data);
	}
}

class wait_for_start : public lobby::dialog
{
public:
	wait_for_start(display& disp, const config& game_config, config& cfg,
			const game_data& data, int team_num, const std::string& team_name,
			const std::string& team_leader)
		: status(START_GAME), got_side(false), team(team_num),
		  name(team_name), leader(team_leader), disp_(disp),
		  sides_(cfg), units_data_(data), game_config_(game_config),
		  cancel_button_(NULL), menu_(NULL)
	{
		SDL_Rect empty_rect = {0,0,0,0};
		area_ = empty_rect;
	}

	void generate_menu(bool first)
	{
		if(area_.h == 0) {
			return;
		}

		std::vector<std::string> details;
		const config* const era_cfg = game_config_.find_child("era","id",sides_["era"]);

		const config::child_list& sides = sides_.get_children("side");
		for(config::child_list::const_iterator s = sides.begin(); s != sides.end(); ++s) {
			const config& sd = **s;

			std::string description = sd["description"];
			std::string side_name = (*(era_cfg->find_child("multiplayer_side", "id", sd["id"])))["name"];
			std::string leader_type = sd["type"];

			if(first && (s - sides.begin() == size_t(team-1))) {
				description = preferences::login();
				side_name = name;
				leader_type = leader;
			}

			std::string leader_name;
			std::string leader_image;
			const game_data::unit_type_map& utypes = units_data_.unit_types;
			if (utypes.find(leader_type) != utypes.end()) {
				leader_name = utypes.find(leader_type)->second.language_name();
				leader_image = utypes.find(leader_type)->second.image();
			}
			if (!leader_image.empty()) {
				// Dumps the "image" part of the faction name, if any,
				// to replace it by a picture of the actual leader
				if(side_name[0] == font::IMAGE) {
					int p = side_name.find_first_of(COLUMN_SEPARATOR);
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

		SDL_Rect rect = area_;
		rect.h -= 2 * gui::ButtonVPadding;
		if (cancel_button_)
			rect.h -= cancel_button_->height();
		SDL_FillRect(disp_.video().getSurface(),&rect,SDL_MapRGB(disp_.video().getSurface()->format,0,0,0));

		menu_.assign(new gui::menu(disp_, details, false, rect.h));
		menu_->set_location(rect);
	}
	
	void set_area(const SDL_Rect& area) {
		area_ = area;

		cancel_button_.assign(new gui::button(disp_,_("Cancel")));
		int y = area.y + area.h - cancel_button_->height() - gui::ButtonVPadding;
		cancel_button_->set_location(area.x + area.w - cancel_button_->width() - gui::ButtonHPadding, y);

		std::string const text = _("Waiting for game to start...");
		SDL_Rect rect = font::draw_text(NULL, disp_.screen_area(), font::SIZE_NORMAL,
		                                font::NORMAL_COLOUR, text, 0, 0);
		rect.x = area_.x + gui::ButtonHPadding;
		rect.y = y + 4;
		font::draw_text(&disp_, rect, font::SIZE_NORMAL, font::NORMAL_COLOUR, text, rect.x, rect.y);

		generate_menu(true);
		events::raise_draw_event();
	}

	void clear_widgets() {
		cancel_button_.assign(NULL);
		menu_.assign(NULL);
	}

	lobby::RESULT process() {
		if (cancel_button_->pressed())
			return lobby::QUIT;

		if(menu_ != NULL) {
			menu_->process();
		}

		config reply;
		const network::connection res = network::receive_data(reply);
		if(res) {
			LOG_NW << "received data while waiting: " << reply.write() << "\n";
			const config::child_list& assigns = reply.get_children("reassign_side");
			for(config::child_list::const_iterator a = assigns.begin(); a != assigns.end(); ++a) {
				if(lexical_cast_default<int>((**a)["from"]) == team) {
					team = lexical_cast_default<int>((**a)["to"]);
					generate_menu(false);
				}
			}

			if(reply.values["failed"] == "yes") {
				status = SIDE_UNAVAILABLE;
				return lobby::QUIT;
			} else if(reply["side_secured"].empty() == false) {
				got_side = true;
				LOG_NW << "received side secured message\n";
			} else if(reply.child("start_game")) {
				LOG_NW << "received start_game message\n";
				//break out of dialog
				status = START_GAME;
				return lobby::CREATE;
			} else if(reply.child("leave_game")) {
				status = GAME_CANCELLED;
				return lobby::QUIT;
			} else if(reply.child("scenario_diff")) {
				LOG_NW << "received diff for scenario....applying...\n";
				sides_.apply_diff(*reply.child("scenario_diff"));
				generate_menu(false);
			} else if(reply.child("side")) {
				sides_ = reply;
				LOG_NW << "got some sides. Current number of sides = "
				       << sides_.get_children("side").size() << ","
				       << reply.get_children("side").size() << "\n";
			} else {
				data_.push_back(reply);
			}
		}

		return lobby::CONTINUE;
	}

	enum { START_GAME, GAME_CANCELLED, SIDE_UNAVAILABLE } status;

	bool got_side;
	int team;

private:
	std::string name;
	std::string leader;
	display& disp_;
	config& sides_;
	const game_data& units_data_;
	const config& game_config_;
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
	log_scope2(network, "playing multiplayer client");

	const network::manager net_manager;

	if(host.empty()) {
		host = preferences::network_host();
		const int res = gui::show_dialog(disp,NULL,_("Connect to Host"),"",
		                                 gui::OK_CANCEL,NULL,NULL,
		                                 _("Choose host to connect to") + std::string(": "),&host);
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

	network::connection data_res = gui::network_data_dialog(disp,_("Connecting to remote host..."),data);
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
				                    _("You must log in to this server"),gui::OK_CANCEL,
									NULL,NULL,_("Login") + std::string(": "),&login);
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
				throw network::error(_("Connection timed out"));
			}

			LOG_NW << "login response: '" << data.write() << "'\n";

			error = data.child("error");
		} while(error != NULL);

		logged_in = true;
	}

	for(bool first_time = true; (first_time || logged_in) && network::nconnections() > 0;
	    first_time = false) {

		if(!first_time) {
			receive_gamelist(disp,data);
		}

		LOG_NW << "when receiving gamelist got '" << data.write() << "'\n";

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
				data_res = gui::network_data_dialog(disp,_("Getting game data..."),sides);
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
		std::string team_leader;
		
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
				gui::show_dialog(disp,NULL,"",_("There are no available sides in this game."),gui::OK_ONLY);
				continue;
			}

			team_num = nchoice;

			config response;
			response["side"] = lexical_cast<std::string>(nchoice);
			response["description"] = preferences::login();

			const std::string& era = sides["era"];

			const config* const era_cfg = cfg.find_child("era","id",era);

			if(era_cfg == NULL) {
				ERR_NW << "era '" << era << "' not found\n";
				return;
			}

			const config::child_list& possible_sides = era_cfg->get_children("multiplayer_side");

			if(possible_sides.empty()) {
				ERR_NW << "no multiplayer sides found\n";
				return;
			}

			size_t choice = 0;

			std::vector<std::string> choices;
			for(config::child_list::const_iterator side =
			    possible_sides.begin(); side != possible_sides.end(); ++side) {
				choices.push_back((**side)["name"]);

				if(choices.back() == default_race) {
					choice = side - possible_sides.begin();
				}
			}

			//if the client is allowed to choose their team, instead of having
			//it set by the server, do that here.
			std::string leader;

			if(allow_changes) {
				std::vector<gui::preview_pane* > preview_panes;
				leader_preview_pane leader_selector(disp, &units_data, possible_sides);
				preview_panes.push_back(&leader_selector);

				choice = size_t(gui::show_dialog(disp,NULL,"",
				     _("Choose your side:"),gui::OK_ONLY,&choices,&preview_panes));
				leader = leader_selector.get_selected_leader();
			}

			wassert(choice < possible_sides.size());

			const config& chosen_side = *possible_sides[choice];
			response["id"] = chosen_side["id"];
			team_name = response["name"] = chosen_side["name"];
			if(leader.empty()) {
				team_leader = response["type"] = chosen_side["type"];
			} else {
				team_leader = response["type"] = leader;
			}
			response["random_faction"] = chosen_side["random_faction"];
			response["recruit"] = chosen_side["recruit"];
			response["music"] = chosen_side["music"];

			network::send_data(response);
		}
    
		wait_for_start waiter(disp,cfg,sides,units_data,team_num,team_name,team_leader);
		std::vector<std::string> messages;
		config game_data;
		lobby::RESULT dialog_res = lobby::CONTINUE;
		while(dialog_res == lobby::CONTINUE) {
			dialog_res = lobby::enter(disp,game_data,cfg,&waiter,messages);
		}

		waiter.clear_widgets();

		if(waiter.status == wait_for_start::GAME_CANCELLED) {
			gui::show_dialog(disp,NULL,"",_("The game has been cancelled"),
			                 gui::OK_ONLY);
			continue;
		} else if(waiter.status == wait_for_start::SIDE_UNAVAILABLE) {
			gui::show_dialog(disp,NULL,"",_("The side you have chosen is no longer available"),
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
			LOG_NW << "setting replay\n";
			recorder = replay(replay_data_store);
			if(!recorder.empty()) {
				const int res = gui::show_dialog(disp,NULL,
		               "", _("Show replay of game up to save point?"),
					   gui::YES_NO);
				//if yes, then show the replay, otherwise
				//skip showing the replay
				if(res == 0) {
					sides = state.starting_pos;
					recorder.set_skip(0);
				} else {
					LOG_NW << "skipping...\n";
					recorder.set_skip(-1);
				}
			}
    
			sides.clear_children("replay");
		}
    
		LOG_NW << "starting game\n";

		state.starting_pos = sides;
		state.snapshot = sides;
		state.players.clear();
    
		recorder.set_save_info(state);
    
		std::vector<config*> story;
		play_level(units_data,cfg,&sides,disp.video(),state,story);
		recorder.clear();
	}
}

leader_list_manager::leader_list_manager(const config::child_list& side_list,
		const game_data* data, gui::combo* combo) :
	side_list_(side_list), data_(data), combo_(combo)
{
#if 0
	for(config::const_child_iterator itor = side_list.begin(); itor != side_list.end(); ++itor) {
		std::cerr << "Child: " << *itor << "\n";
		std::cerr << (**itor).write();
	}
#endif
}

void leader_list_manager::set_combo(gui::combo* combo)
{
	combo_ = combo;

	if (combo_ != NULL) {
		update_leader_list(0);
	}
}

void leader_list_manager::update_leader_list(int side_index)
{
	const config& side = *side_list_[side_index];

	leaders_.clear();

	if(side["random_faction"] == "yes") {
		if(combo_ != NULL) {
			std::vector<std::string> dummy;
			dummy.push_back("-");
			combo_->enable(false);
			combo_->set_items(dummy);
			combo_->set_selected(0);
		}
		return;
	} else {
		if(combo_ != NULL)
			combo_->enable(true);
	}

	if(!side["leader"].empty()) {
		leaders_ = config::split(side["leader"]);
	}
	
	const std::string default_leader = side["type"];
	int default_index = 0;

	std::vector<std::string>::const_iterator itor;

	for (itor = leaders_.begin(); itor != leaders_.end(); ++itor) {
		if (*itor == default_leader) {
			break;
		}
		default_index++;
	}

	if (default_index == leaders_.size()) {
		leaders_.push_back(default_leader);
	}

	std::vector<std::string> leader_strings;

	for(itor = leaders_.begin(); itor != leaders_.end(); ++itor) {
		
		const game_data::unit_type_map& utypes = data_->unit_types;

		//const std::string name = data_->unit_types->find(*itor).language_name();
		if (utypes.find(*itor) != utypes.end()) {
			const std::string name =  utypes.find(*itor)->second.language_name();
			const std::string image = utypes.find(*itor)->second.image();

			leader_strings.push_back(IMAGE_PREFIX + image + COLUMN_SEPARATOR + name);
		} else {
			leader_strings.push_back("?");
		}
	}

	leaders_.push_back("random");
	// FIXME: Maybe this should not code into the code.
	leader_strings.push_back(IMAGE_PREFIX + std::string("random-enemy.png") +
	                         COLUMN_SEPARATOR + _("Random"));

	if(combo_ != NULL) {
		combo_->set_items(leader_strings);
		combo_->set_selected(default_index);
	}
}

void leader_list_manager::set_leader(const std::string& leader)
{
	if(combo_ == NULL)
		return;

	int leader_index = 0;
	for(std::vector<std::string>::const_iterator itor = leaders_.begin();
			itor != leaders_.end(); ++itor) {
		if (leader == *itor) {
			combo_->set_selected(leader_index);
			return;
		}
		++leader_index;
	}
}

std::string leader_list_manager::get_leader()
{
	if(combo_ == NULL)
		return _("?");

	if(combo_->selected() >= leaders_.size())
		return _("?");

	return leaders_[combo_->selected()];
}

namespace {
	static const SDL_Rect leader_pane_position = {-260,-370,260,370};
	static const int leader_pane_border = 10;
}

leader_preview_pane::leader_preview_pane(display& disp, const game_data* data,
		const config::child_list& side_list) :
	gui::preview_pane(disp),
	side_list_(side_list),
	leader_combo_(disp, std::vector<std::string>()), 
	leaders_(side_list, data, &leader_combo_),
	selection_(0), data_(data)
{

	set_location(leader_pane_position);
}

void leader_preview_pane::process_event()
{
	if (leader_combo_.changed()) {
		set_dirty();
	}
}

void leader_preview_pane::draw_contents()
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
		const std::vector<std::string> recruit_list = config::split(recruits);
		std::ostringstream recruit_string;

		if(faction[0] == font::IMAGE) {
			int p = faction.find_first_of(COLUMN_SEPARATOR);
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

bool leader_preview_pane::show_above() const
{
	return false;
}

bool leader_preview_pane::left_side() const
{
	return false;
}

void leader_preview_pane::set_selection(int selection)
{
	selection_ = selection;
	leaders_.update_leader_list(selection_);
	set_dirty();
}

std::string leader_preview_pane::get_selected_leader()
{
	return leaders_.get_leader();
}
