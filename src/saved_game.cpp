

#include "saved_game.hpp"
#include "gamestatus.hpp"
#include "carryover.hpp"
#include "log.hpp"
#include "statistics.hpp"
#include "serialization/binary_or_text.hpp"
#include "util.hpp"

#include <boost/foreach.hpp>

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

saved_game::saved_game(const config& cfg) 
	: replay_data()
	, snapshot()
	, carryover_sides()
	, carryover_sides_start()
	, replay_start_()
	, classification_(cfg)
	, mp_settings_(cfg)
{
	log_scope("read_game");
	
	carryover_sides = cfg.child_or_empty("carryover_sides");
	carryover_sides_start = cfg.child_or_empty("carryover_sides_start");
	replay_start_ = cfg.child_or_empty("replay_start");
	replay_data = cfg.child_or_empty("replay");
	snapshot = cfg.child_or_empty("snapshot");

	if(snapshot.empty() && carryover_sides_start.empty() && !carryover_sides.empty())
	{
		//Explain me: when could this happen?
		//if we are loading a start of scenario save and don't have carryover_sides_start, use carryover_sides
		carryover_sides_start = carryover_sides;
	}

	LOG_NG << "scenario: '" << carryover_sides_start["next_scenario"].str() << "'\n";

	if (const config &stats = cfg.child("statistics")) {
		statistics::fresh_stats();
		statistics::read_stats(stats);
	}

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
