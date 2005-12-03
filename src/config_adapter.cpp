#include "global.hpp"

#include <sstream>
#include "config_adapter.hpp"
#include "game_errors.hpp"
#include "log.hpp"
#include "preferences.hpp"

#define LOG_NG LOG_STREAM(info, engine)

std::string get_unique_saveid(const config& cfg, std::set<std::string>& seen_save_ids){
	std::string save_id = cfg["save_id"];

	if(save_id.empty()) {
		save_id=cfg["description"];
	}

	if(save_id.empty()) {
		save_id="Unknown";
	}

	//make sure the 'save_id' is unique
	while(seen_save_ids.count(save_id)) {
		save_id += "_";
	}

	return save_id;
}

void get_player_info(const config& cfg, game_state& gamestate, std::string save_id, std::vector<team>& teams, const config& level, const game_data& gameinfo, gamemap& map, unit_map& units){
	player_info *player = NULL;

	if(cfg["controller"] == "human" ||
	   cfg["controller"] == "network" ||
	   cfg["persistent"] == "1") {
		player = gamestate.get_player(save_id);

		if(player == NULL && !save_id.empty()) {
			player = &gamestate.players[save_id];
		}
	}

	LOG_NG << "initializing team...\n";

	std::string gold = cfg["gold"];
	if(gold.empty())
		gold = "100";

	LOG_NG << "found gold: '" << gold << "'\n";

	int ngold = lexical_cast_default<int>(gold);
	if(player != NULL && player->gold >= ngold) {
		ngold = player->gold;
	}

	LOG_NG << "set gold to '" << ngold << "'\n";

	teams.push_back(team(cfg,ngold));

	//update/fix the recall list for this side, by setting the
	//"side" of each unit in it to be the "side" of the player.
	int side = lexical_cast_default<int>(cfg["side"], 1);
	if(player != NULL) {
		for(std::vector<unit>::iterator it = player->available_units.begin(); it != player->available_units.end(); ++it) {
			it->set_side(side);
		}
	}

	//if this team has no objectives, set its objectives to the
	//level-global "objectives"
	if(teams.back().objectives().empty())
		teams.back().set_objectives(level["objectives"]);

	//if this side tag describes the leader of the side
	if(cfg["no_leader"] != "yes" && cfg["controller"] != "null") {
		unit new_unit(gameinfo, cfg);

		//search the recall list for leader units, and if there is
		//one, use it in place of the config-described unit
		if(player != NULL) {
			for(std::vector<unit>::iterator it = player->available_units.begin(); it != player->available_units.end(); ++it) {
				if(it->can_recruit()) {
					new_unit = *it;
					player->available_units.erase(it);
					break;
				}
			}
		}

		//see if the side specifies its location. Otherwise start it at the map-given
		//starting position
		gamemap::location start_pos(cfg);

		if(map.empty()) {
			throw game::load_game_failed("Map not found");
		}
		if(cfg["x"].empty() && cfg["y"].empty()) {
			start_pos = map.starting_position(side);
		}

		if(!start_pos.valid() || !map.on_board(start_pos)) {
			throw game::load_game_failed(
				"Invalid starting position (" +
				lexical_cast<std::string>(start_pos.x+1) +
				"," + lexical_cast<std::string>(start_pos.y+1) +
				") for the leader of side " +
				lexical_cast<std::string>(side) + ".");
		}

		new_unit.new_turn();
		units.insert(std::pair<gamemap::location,unit>(
				map.starting_position(new_unit.side()), new_unit));
		LOG_NG << "initializing side '" << cfg["side"] << "' at "
			<< start_pos << '\n';
	}

	//if the game state specifies units that can be recruited for the player
	//then add them
	if(player != NULL && player->can_recruit.empty() == false) {
		std::copy(player->can_recruit.begin(),player->can_recruit.end(),
				std::inserter(teams.back().recruits(),teams.back().recruits().end()));
	}

	if(player != NULL) {
		player->can_recruit = teams.back().recruits();
	}

	//if there are additional starting units on this side
	const config::child_list& starting_units = cfg.get_children("unit");
	for(config::child_list::const_iterator su = starting_units.begin(); su != starting_units.end(); ++su) {
		unit new_unit(gameinfo,**su);

		new_unit.set_side(side);

		const std::string& x = (**su)["x"];
		const std::string& y = (**su)["y"];

		const gamemap::location loc(**su);
		if(x.empty() && y.empty()) {
			if(player) {
				player->available_units.push_back(new_unit);
				LOG_NG << "inserting unit on recall list for side " << new_unit.side() << "\n";
			} else {
				throw game::load_game_failed(
					"Attempt to create a unit on the recall list for side " +
					lexical_cast<std::string>(side) +
					", which does not have a recall list.");
			}
		} else if(!loc.valid() || !map.on_board(loc)) {
			throw game::load_game_failed(
				"Invalid starting position (" +
				lexical_cast<std::string>(loc.x+1) +
				"," + lexical_cast<std::string>(loc.y+1) +
				") for a unit on side " +
				lexical_cast<std::string>(side) + ".");
		} else {
			units.insert(std::pair<gamemap::location,unit>(loc,new_unit));
			LOG_NG << "inserting unit for side " << new_unit.side() << "\n";
		}
	}
}

int get_first_human_team(const config::child_list::const_iterator& cfg, const config::child_list& unit_cfg){
	int result = -1;
	const std::string& controller = (**cfg)["controller"];
	if (controller == preferences::client_type() && (**cfg)["description"] == preferences::login()) {
		result = cfg - unit_cfg.begin();
	} else if(result == -1 && ((**cfg)["controller"] == "human" || (**cfg)["persistent"] == "1")) {
		result = cfg - unit_cfg.begin();
	}
	return result;
}

const config* get_theme(const config& game_config, std::string theme_name){
	const config* theme_cfg = NULL;
	if(theme_name != "") {
		theme_cfg = game_config.find_child("theme","name",theme_name);
	}

	if(theme_cfg == NULL) {
		theme_cfg = game_config.find_child("theme","name",preferences::theme());
	}
	if (theme_cfg == NULL){
		theme_cfg = new config();
	}
	return theme_cfg;
}

