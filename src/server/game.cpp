#include "../global.hpp"

#include "game.hpp"
#include "../util.hpp"
#include "../wassert.hpp"

#include <iostream>

int game::id_num = 1;

game::game(const player_map& info) : player_info_(&info), id_(id_num++), started_(false), description_(NULL), end_turn_(0), allow_observers_(true)
{}

bool game::is_owner(network::connection player) const
{
	wassert(!players_.empty());
	return player == players_.front();
}

bool game::is_member(network::connection player) const
{
	return std::find(players_.begin(),players_.end(),player) != players_.end();
}

bool game::is_needed(network::connection player) const
{
	wassert(!players_.empty());
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
	return false;
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

			if(observers_can_label() && (*i)->child("label") != NULL && (*i)->all_children().size() == 1) {
				;
			} else if(observers_can_chat() && (*i)->child("speak") != NULL && (*i)->all_children().size() == 1) {
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

namespace {
std::string describe_turns(int turn, const std::string& num_turns)
{
	char buf[50];
	sprintf(buf,"%d/",int(turn));

	if(num_turns == "-1") {
		return buf + std::string("-");
	} else {
		return buf + num_turns;
	}
}

}

void game::start_game()
{
	started_ = true;

	describe_slots();
	if(description()) {
		description()->values["turn"] = describe_turns(1,level()["turns"]);
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
	wassert(is_member(player));

	const std::string& side = cfg["side"];
	
	//if the player is already on a side
	if(sides_.count(player))
		return false;

	//if the side is already taken, see if we can give the player
	//another side instead
	if(sides_taken_.count(side)) {
		std::cerr << "side '" << side << "' taken, searching for alternative side\n";
		const config::child_list& sides = level_.get_children("side");
		for(config::child_list::const_iterator i = sides.begin(); i != sides.end(); ++i) {
			if((**i)["controller"] == "network" && sides_taken_.count((**i)["side"]) == 0) {
				config new_cfg = cfg;
				new_cfg["side"] = (**i)["side"];

				const bool res = take_side(player,new_cfg);

				//if there's another side available, then tell the player that they've
				//had their side reassigned
				if(res) {
					config response;
					config& reassign = response.add_child("reassign_side");
					reassign["from"] = cfg["side"];
					reassign["to"] = new_cfg["side"];
					network::queue_data(response,player);
				}

				return res;
			}
		}
	}

	sides_[player] = side;
	sides_taken_.insert(side);

	//send host notification of taking this side
	if(players_.empty() == false) {
		network::queue_data(cfg,players_.front());
	}

	return true;
}

const std::string& game::transfer_side_control(const config& cfg)
{
	const std::string& player = cfg["player"];

	std::vector<network::connection>::const_iterator i;
	for(i = players_.begin(); i != players_.end(); ++i) {
		const player_map::const_iterator pl = player_info_->find(*i);
		if(pl != player_info_->end() && pl->second.name() == player) {
			break;
		}
	}

	if(i == players_.end()) {
		static const std::string notfound = "Player not found";
		return notfound;
	}

	const std::string& side = cfg["side"];

	for(std::map<network::connection,std::string>::const_iterator j = sides_.begin(); j != sides_.end(); ++j) {
		if(j->second == side) {
			static const std::string already = "This side is already controlled by a player";
			return already;
		}
	}

	if(sides_.count(*i) == 1) {
		static const std::string already = "Player already controls a side";
		return already;
	}

	if(cfg["orphan_side"] != "yes") {
		const size_t nsides = level_.get_children("side").size();
		const size_t active_side = end_turn_/nsides + 1;

		if(lexical_cast_default<size_t>(side,0) == active_side) {
			static const std::string not_during_turn = "You cannot change a side's controller during its turn";
			return not_during_turn;
		}
	}

	config response;
	config& change = response.add_child("change_controller");

	change["side"] = side;
	change["controller"] = "network";

	network::queue_data(response,players_.front());

	change["controller"] = "human";
	network::queue_data(response,*i);

	if(i != players_.begin()) {
		sides_[*i] = side;
	}

	sides_taken_.insert(side);
	
	//send everyone a message saying that the observer who is taking the side has quit
	config observer_quit;
	observer_quit.add_child("observer_quit").values["name"] = player;
	send_data(observer_quit);

	static const std::string success = "";
	return success;
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
		description()->values["observer"] = level_["observer"];
		return true;
	} else {
		return false;
	}
}

bool game::player_is_banned(network::connection sock) const
{
	if(bans_.empty()) {
		return false;
	}

	const player_map::const_iterator itor = player_info_->find(sock);
	if(itor == player_info_->end()) {
		return false;
	}

	const player& info = itor->second;
	const std::string& ipaddress = network::ip_address(sock);
	for(std::vector<ban>::const_iterator i = bans_.begin(); i != bans_.end(); ++i) {
		if(info.name() == i->username || ipaddress == i->ipaddress) {
			return true;
		}
	}

	return false;
}

void game::ban_player(network::connection sock)
{
	const player_map::const_iterator itor = player_info_->find(sock);
	if(itor != player_info_->end()) {
		bans_.push_back(ban(itor->second.name(),network::ip_address(sock)));
	}

	remove_player(sock);
}

bool game::process_commands(const config& cfg)
{
	//std::cerr << "processing commands: '" << cfg.write() << "'\n";
	bool res = false;
	const config::child_list& cmd = cfg.get_children("command");
	for(config::child_list::const_iterator i = cmd.begin(); i != cmd.end(); ++i) {
		if((**i).child("end_turn") != NULL) {
			res = res || end_turn();
			//std::cerr << "res: " << (res ? "yes" : "no") << "\n";
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

	desc->values["turn"] = describe_turns(int(turn),level()["turns"]);

	return true;
}

void game::add_player(network::connection player)
{
	//if the game has already started, we add the player as an observer
	if(started_) {
		if(!allow_observers_) {
			return;
		}

		const player_map::const_iterator info = player_info_->find(player);
		if(info != player_info_->end()) {
			config observer_join;
			observer_join.add_child("observer").values["name"] = info->second.name();
			send_data(observer_join);
		}

		//tell this player that the game has started
		config cfg;
		cfg.add_child("start_game");
		network::queue_data(cfg,player);
	}

	//if the player is already in the game, don't add them.
	if(std::find(players_.begin(),players_.end(),player) != players_.end()) {
		return;
	}

	players_.push_back(player);

	send_user_list();

	//send the player the history of the game to-date
	network::queue_data(history_,player);
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
			network::queue_data(drop,players_.front());
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
		if(*i != exclude && (allow_observers_ || is_needed(*i) || sides_.count(*i) == 1)) {
			network::queue_data(data,*i);
		}
	}
}

bool game::player_on_team(const std::string& team, network::connection player) const
{
	//if the player is the game host, then iterate over all the sides and if any of
	//the sides controlled by the game host are on this team, then the game host
	//is on this team
	if(players_.empty() == false && player == players_.front()) {
		const config::child_list& sides = level_.get_children("side");
		for(config::child_list::const_iterator i = sides.begin(); i != sides.end(); ++i) {
			if((**i)["controller"] == "human" && (**i)["team_name"] == team) {
				return true;
			}
		}
	}

	//hosts other than the game host
	const std::map<network::connection,std::string>::const_iterator side = sides_.find(player);
	if(side != sides_.end()) {
		const config* const side_cfg = level_.find_child("side","side",side->second);
		if(side_cfg != NULL && (*side_cfg)["team_name"] == team) {
			return true;
		}
	}

	return false;
}

void game::send_data_team(const config& data, const std::string& team, network::connection exclude)
{
	for(std::vector<network::connection>::const_iterator i = players_.begin(); i != players_.end(); ++i) {
		if(*i != exclude && player_on_team(team,*i)) {
			network::queue_data(data,*i);
		}
	}
}

void game::send_data_observers(const config& data)
{
	for(std::vector<network::connection>::const_iterator i = players_.begin(); i != players_.end(); ++i) {
		if(is_observer(*i)) {
			network::queue_data(data,*i);
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
