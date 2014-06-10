

#include "saved_game.hpp"
#include "gamestatus.hpp"
#include "carryover.hpp"
#include "log.hpp"
#include "statistics.hpp"
#include "serialization/binary_or_text.hpp"
#include "util.hpp"

#include <boost\foreach.hpp>

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)
#define WRN_NG LOG_STREAM(warn, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)
#define DBG_NG LOG_STREAM(debug, log_engine)

static lg::log_domain log_enginerefac("enginerefac");
#define LOG_RG LOG_STREAM(info, log_enginerefac)

saved_game::saved_game()  :
		replay_data(),
		snapshot(),
		carryover_sides(),
		carryover_sides_start(carryover_info().to_config()),
		replay_start_(),
		classification_(),
		mp_settings_()
		{}

saved_game::saved_game(const config& cfg, bool show_replay) :
		replay_data(),
		snapshot(),
		carryover_sides(),
		carryover_sides_start(),
		replay_start_(),
		classification_(cfg),
		mp_settings_(cfg)
{
	log_scope("read_game");

	if(cfg.has_child("carryover_sides")){
		carryover_sides = cfg.child("carryover_sides");
	}
	if(cfg.has_child("carryover_sides_start")){
		carryover_sides_start = cfg.child("carryover_sides_start");
	}

	if(show_replay){
		//If replay_start and replay_data couldn't be loaded
		if(!load_replay(cfg)){
			//TODO: notify user of failure
			ERR_NG<<"Could not load as replay " << std::endl;
		}
	} else {
		if(const config& snapshot = cfg.child("snapshot")){
			this->snapshot = snapshot;
			load_replay(cfg);
		} else if(carryover_sides_start.empty() && !carryover_sides.empty()){
			//if we are loading a start of scenario save and don't have carryover_sides_start, use carryover_sides
			carryover_sides_start = carryover_sides;
		}
		//TODO: check if loading fails completely
	}

	LOG_NG << "scenario: '" << carryover_sides_start["next_scenario"] << "'\n";

	if (const config &stats = cfg.child("statistics")) {
		statistics::fresh_stats();
		statistics::read_stats(stats);
	}

}

bool saved_game::load_replay(const config& cfg){
	bool replay_loaded = false;

	if(const config& replay_start = cfg.child("replay_start")){
		replay_start_ = replay_start;
		if(const config& replay = cfg.child("replay")){
			this->replay_data = replay;
			replay_loaded = true;
		}
	}

	return replay_loaded;
}

static void convert_old_saves(config& cfg){
	if(!cfg.has_child("snapshot")){
		return;
	}

	const config& snapshot = cfg.child("snapshot");
	const config& replay_start = cfg.child("replay_start");
	const config& replay = cfg.child("replay");

	if(!cfg.has_child("carryover_sides") && !cfg.has_child("carryover_sides_start")){
		config carryover;
		//copy rng and menu items from toplevel to new carryover_sides
		carryover["random_seed"] = cfg["random_seed"];
		carryover["random_calls"] = cfg["random_calls"];
		BOOST_FOREACH(const config& menu_item, cfg.child_range("menu_item")){
			carryover.add_child("menu_item", menu_item);
		}
		carryover["difficulty"] = cfg["difficulty"];
		carryover["random_mode"] = cfg["random_mode"];
		//the scenario to be played is always stored as next_scenario in carryover_sides_start
		carryover["next_scenario"] = cfg["scenario"];

		config carryover_start = carryover;

		//copy sides from either snapshot or replay_start to new carryover_sides
		if(!snapshot.empty()){
			BOOST_FOREACH(const config& side, snapshot.child_range("side")){
				carryover.add_child("side", side);
			}
			//for compatibility with old savegames that use player instead of side
			BOOST_FOREACH(const config& side, snapshot.child_range("player")){
				carryover.add_child("side", side);
			}
			//save the sides from replay_start in carryover_sides_start
			BOOST_FOREACH(const config& side, replay_start.child_range("side")){
				carryover_start.add_child("side", side);
			}
			//for compatibility with old savegames that use player instead of side
			BOOST_FOREACH(const config& side, replay_start.child_range("player")){
				carryover_start.add_child("side", side);
			}
		} else if (!replay_start.empty()){
			BOOST_FOREACH(const config& side, replay_start.child_range("side")){
				carryover.add_child("side", side);
				carryover_start.add_child("side", side);
			}
			//for compatibility with old savegames that use player instead of side
			BOOST_FOREACH(const config& side, replay_start.child_range("player")){
				carryover.add_child("side", side);
				carryover_start.add_child("side", side);
			}
		}

		//get variables according to old hierarchy and copy them to new carryover_sides
		if(!snapshot.empty()){
			if(const config& variables = snapshot.child("variables")){
				carryover.add_child("variables", variables);
				carryover_start.add_child("variables", replay_start.child_or_empty("variables"));
			} else if (const config& variables = cfg.child("variables")){
				carryover.add_child("variables", variables);
				carryover_start.add_child("variables", variables);
			}
		} else if (!replay_start.empty()){
			if(const config& variables = replay_start.child("variables")){
				carryover.add_child("variables", variables);
				carryover_start.add_child("variables", variables);
			}
		} else {
			carryover.add_child("variables", cfg.child("variables"));
			carryover_start.add_child("variables", cfg.child("variables"));
		}

		cfg.add_child("carryover_sides", carryover);
		cfg.add_child("carryover_sides_start", carryover_start);
	}

	//if replay and snapshot are empty we've got a start of scenario save and don't want replay_start either
	if(replay.empty() && snapshot.empty()){
		LOG_RG<<"removing replay_start \n";
		cfg.remove_child("replay_start", 0);
	}

	//remove empty replay or snapshot so type of save can be detected more easily
	if(replay.empty()){
		LOG_RG<<"removing replay \n";
		cfg.remove_child("replay", 0);
	}

	if(snapshot.empty()){
		LOG_RG<<"removing snapshot \n";
		cfg.remove_child("snapshot", 0);
	}

	LOG_RG<<"cfg after conversion "<<cfg<<"\n";
}

saved_game::saved_game(const saved_game& state) :
	replay_data(state.replay_data),
	snapshot(state.snapshot),
	carryover_sides(state.carryover_sides),
	carryover_sides_start(state.carryover_sides_start),
	replay_start_(state.replay_start_),
	classification_(state.classification_),
	mp_settings_(state.mp_settings_)
{}

saved_game& saved_game::operator=(const saved_game& state)
{
	// Use copy constructor to make sure we are coherent
	if (this != &state) {
		this->~saved_game();
		new (this) saved_game(state) ;
	}
	return *this ;
}


void saved_game::write_config(config_writer& out) const
{
	out.write(classification_.to_config());
	if (classification_.campaign_type == game_classification::MULTIPLAYER) {
		out.write_child(lexical_cast<std::string>(game_classification::MULTIPLAYER), mp_settings_.to_config());
	}
}
