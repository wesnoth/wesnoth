#include "game.hpp"

#include <cassert>
#include <iostream>

int game::id_num = 1;

game::game(const player_map& info) : player_info_(&info), id_(id_num++), started_(false), description_(NULL), end_turn_(0), allow_observers_(true)
{}

bool game::is_member(network::connection player) const
{
	return std::find(players_.begin(),players_.end(),player) != players_.end();
}

bool game::is_needed(network::connection player) const
{
	assert(!players_.empty());
	return player == players_.front();
}

bool game::is_observer(network::connection player) const
{
	if(is_member(player) == false) {
		return false;
	}

	return is_needed(player) == false && sides_.count(player) == 0;
}

bool game::observers_can_label() const
{
	return true;
}

bool game::observers_can_chat() const
{
	return true;
}

bool game::filter_commands(network::connection player, config& cfg)
{
	if(is_observer(player)) {
		std::vector<int> marked;
		int index = 0;

		const config::child_list& children = cfg.get_children("command");
		for(config::child_list::const_iterator i = children.begin(); i != children.end(); ++i) {

			if(observers_can_label() && (*i)->child("label") != NULL) {
				;
			} else if(observers_can_chat() && (*i)->child("speak") != NULL) {
				;
			} else {
				std::cerr << "removing observer's illegal command: '" << (*i)->write() << "'\n";
				marked.push_back(index - marked.size());
			}

			++index;
		}

		for(std::vector<int>::const_iterator j = marked.begin(); j != marked.end(); ++j) {
			cfg.remove_child("command",*j);
		}

		return cfg.all_children().empty() == false;
	}

	return true;
}

void game::start_game()
{
	started_ = true;

	describe_slots();
	if(description()) {
		description()->values["turn"] = "1";
	}

	allow_observers_ = level_["observer"] != "no";

	//send [observer] tags for all observers that have already joined.
	//we can do this by re-joining all players that don't have sides, and aren't
	//the first player (game creator)
	for(std::vector<network::connection>::const_iterator pl = players_.begin()+1; pl != players_.end(); ++pl) {
		if(sides_.count(*pl) == 0) {
			add_player(*pl);
		}
	}
}

bool game::take_side(network::connection player, const config& cfg)
{
	assert(is_member(player));

	const std::string& side = cfg["side"];
	
	//if the player is already on a side or the side is already taken
	if(sides_.count(player) || sides_taken_.count(side))
		return false;

	sides_[player] = side;
	sides_taken_.insert(side);

	//send host notification of taking this side
	if(players_.empty() == false) {
		network::send_data(cfg,players_.front());
	}

	return true;
}

size_t game::available_slots() const
{
	size_t total_slots = 0;
	const config::child_list& sides = level_.get_children("side");
	for(config::child_list::const_iterator i = sides.begin(); i != sides.end(); ++i) {
		std::cerr << "side controller: '" << (**i)["controller"] << "'\n";
		if((**i)["controller"] == "network") {
			++total_slots;
		}
	}

	return total_slots - sides_taken_.size();
}

bool game::describe_slots()
{
	if(description() == NULL)
		return false;

	const int val = int(available_slots());
	char buf[50];
	sprintf(buf,"%d",val);

	if(buf != (*description())["slots"]) {
		description()->values["slots"] = buf;
		return true;
	} else {
		return false;
	}
}

bool game::process_commands(const config& cfg)
{
	std::cerr << "processing commands: '" << cfg.write() << "'\n";
	bool res = false;
	const config::child_list& cmd = cfg.get_children("command");
	for(config::child_list::const_iterator i = cmd.begin(); i != cmd.end(); ++i) {
		if((**i).child("end_turn") != NULL) {
			res = res || end_turn();
			std::cerr << "res: " << (res ? "yes" : "no") << "\n";
		}
	}

	return res;
}

bool game::end_turn()
{
	//it's a new turn every time each side in the game ends their turn.
	++end_turn_;

	const size_t nsides = level_.get_children("side").size();

	if((end_turn_%nsides) != 0) {
		return false;
	}

	const size_t turn = end_turn_/nsides + 1;

	config* const desc = description();
	if(desc == NULL) {
		return false;
	}

	char buf[50];
	sprintf(buf,"%d",int(turn));
	desc->values["turn"] = buf;

	return true;
}

void game::add_player(network::connection player)
{
	//if the game has already started, we add the player as an observer
	if(started_) {
		const player_map::const_iterator info = player_info_->find(player);
		if(info != player_info_->end()) {
			config observer_join;
			observer_join.add_child("observer").values["name"] = info->second.name();
			send_data(observer_join);
		}

		//tell this player that the game has started
		config cfg;
		cfg.add_child("start_game");
		network::send_data(cfg,player);
	}

	//if the player is already in the game, don't add them.
	if(std::find(players_.begin(),players_.end(),player) != players_.end()) {
		return;
	}

	players_.push_back(player);

	send_user_list();

	//send the player the history of the game to-date
	network::send_data(history_,player);
}

void game::remove_player(network::connection player)
{
	const std::vector<network::connection>::iterator itor =
	             std::find(players_.begin(),players_.end(),player);
	if(itor != players_.end())
		players_.erase(itor);

	bool observer = true;
	std::map<network::connection,std::string>::iterator side = sides_.find(player);
	if(side != sides_.end()) {
		//send the host a notification of removal of this side
		if(players_.empty() == false) {
			config drop;
			drop["side_drop"] = side->second;
			network::send_data(drop,players_.front());
		}

		sides_taken_.erase(side->second);
		sides_.erase(side);

		observer = false;
	}

	send_user_list();

	const player_map::const_iterator pl = player_info_->find(player);
	if(!observer || pl == player_info_->end()) {
		return;
	}

	//they're just an observer, so send them having quit to clients
	config observer_quit;
	observer_quit.add_child("observer_quit").values["name"] = pl->second.name();
	send_data(observer_quit);
}

void game::send_user_list()
{
	//if the game hasn't started yet, then send all players a list
	//of the users in the game
	if(started_ == false && description() != NULL) {
		config cfg;
		cfg.add_child("gamelist");
		for(std::vector<network::connection>::const_iterator p = players_.begin(); p != players_.end(); ++p) {
			
			const player_map::const_iterator info = player_info_->find(*p);
			if(info != player_info_->end()) {
				config& user = cfg.add_child("user");
				user["name"] = info->second.name();
			}
		}

		send_data(cfg);
	}
}


int game::id() const
{
	return id_;
}

void game::send_data(const config& data, network::connection exclude)
{
	for(std::vector<network::connection>::const_iterator
	    i = players_.begin(); i != players_.end(); ++i) {
		if(*i != exclude && (allow_observers_ || sides_.count(*i) == 1)) {
			network::send_data(data,*i);
		}
	}
}

void game::send_data_team(const config& data, const std::string& team, network::connection exclude)
{
	for(std::vector<network::connection>::const_iterator
	    i = players_.begin(); i != players_.end(); ++i) {
		if(*i != exclude && sides_.count(*i) == 1) {
			const config* const side = level_.find_child("side","side",sides_[*i]);
			if(side != NULL && (*side)["team_name"] == team) {
				network::send_data(data,*i);
			}
		}
	}
}

void game::record_data(const config& data)
{
	history_.append(data);
}

bool game::level_init() const
{
	return level_.child("side") != NULL;
}

config& game::level()
{
	return level_;
}

bool game::empty() const
{
	return players_.empty();
}

void game::disconnect()
{
	for(std::vector<network::connection>::iterator i = players_.begin();
	    i != players_.end(); ++i) {
		network::queue_disconnect(*i);
	}

	players_.clear();
}

void game::set_description(config* desc)
{
	description_ = desc;
}

config* game::description()
{
	return description_;
}

void game::add_players(const game& other_game)
{
	players_.insert(players_.end(),
	                other_game.players_.begin(),other_game.players_.end());
}

bool game::started() const
{
	return started_;
}
