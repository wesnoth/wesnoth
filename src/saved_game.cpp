/**
	Some information about savefiles:
	A saveile can contain: 
	 * General information (toplevel atributes, [multiplayer])
	    This is present in all savefiles
	 * [statistics]
	    This is present in all savefiles but it's not handled by playcampaign/play_controller/saved_game.
		It's handled by savegame.cpp
	 * [snapshot]
	    If a savegame was saved during a scenaio this contzaings a snapshot of teh game at the point when it was saved.
	 * [carryover_sides_start]
	    At start-of-scenrio saves this contains data from the previous scenario that was preserved
	 * [carryover_sides]
	    In savefile made during the game, this tag contains data from [carryover_sides_start] that was not used in the current scenario but should be saved for a next scenario
	 * [replay_start]
	    A snapshot made very early to replay the game from.
	 * [replay]
	    A record of game actions that was made between the creation of [replay_start] and [snapshot].



	The following types of savegames are known:
	 * Start of scenario savefiles
	    These files only contain general information, statistics, and [carryover_sides_start]
		When these saves are loaded the scenario is loaded form teh game config by the next_scenario attribute from [carryover_sides_start]
	 * Expanded Start of scenario savefiles
	    Similar to normal Start-of-scenario savefiles, but the also contain a [scenario] that contins the scenario
		This type is only used internaly and usualy doesn't get written to the disk.
	 * Ingame savefile
	    These files contain genral information, statistics, [snapshot], [replay], [replay_start], [snapshot], [carryover_sides]
		Thedse files don't contain a [carryover_sides_start] because both starting points ([replay_start] and [snapshot]) 
		were made after [carryover_sides_start] was merged into the scenario.
	 * Replay savefiles
	    Like a Ingame save made durign linger mode, but without the [snapshot]
*/

#include "saved_game.hpp"
#include "gamestatus.hpp"
#include "carryover.hpp"
#include "log.hpp"
#include "resources.hpp"
#include "game_config_manager.hpp"
#include "statistics.hpp"
#include "serialization/binary_or_text.hpp"
#include "util.hpp"

#include <boost/foreach.hpp>
#include <cassert>

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)
#define WRN_NG LOG_STREAM(warn, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)
#define DBG_NG LOG_STREAM(debug, log_engine)

saved_game::saved_game()
	: replay_data()
	, carryover_sides()
	, carryover_sides_start(carryover_info().to_config())
	, replay_start_()
	, classification_()
	, mp_settings_()
	, starting_pos_type_(STARTINGPOS_NONE)
	, starting_pos_()
{

}

saved_game::saved_game(const config& cfg) 
	: replay_data()
	, carryover_sides()
	, carryover_sides_start()
	, replay_start_()
	, classification_(cfg)
	, mp_settings_(cfg)
	, starting_pos_type_(STARTINGPOS_NONE)
	, starting_pos_()
	
{
	log_scope("read_game");

	carryover_sides = cfg.child_or_empty("carryover_sides");
	carryover_sides_start = cfg.child_or_empty("carryover_sides_start");
	replay_start_ = cfg.child_or_empty("replay_start");
	replay_data = cfg.child_or_empty("replay");
	if(const config& snapshot = cfg.child("snapshot"))
	{
		this->starting_pos_type_ = STARTINGPOS_SNAPSHOT;
		this->starting_pos_ = snapshot;
	}
	else if(const config& scenario = cfg.child("scenario"))
	{
		this->starting_pos_type_ = STARTINGPOS_SCENARIO;
		this->starting_pos_ = scenario;
	}

	if(starting_pos_.empty() && carryover_sides_start.empty() && !carryover_sides.empty())
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

saved_game::saved_game(const saved_game& state) 
	: replay_data(state.replay_data)
	, carryover_sides(state.carryover_sides)
	, carryover_sides_start(state.carryover_sides_start)
	, replay_start_(state.replay_start_)
	, classification_(state.classification_)
	, mp_settings_(state.mp_settings_)
	, starting_pos_type_(state.starting_pos_type_)
	, starting_pos_(state.starting_pos_)
{
}

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
	write_general_info(out);
	write_starting_pos(out);
	if(!this->replay_start_.empty())
	{
		out.write_child("replay_start", replay_start_);	
	}
	if(!this->replay_data.empty())
	{
		out.write_child("replay", replay_data);	
	}
	write_carryover(out);
}

void saved_game::write_starting_pos(config_writer& out) const
{
	if(starting_pos_type_ == STARTINGPOS_SNAPSHOT)
	{
		out.write_child("snapshot", starting_pos_);
	}
	else if(starting_pos_type_ == STARTINGPOS_SCENARIO)
	{
		out.write_child("scenario", starting_pos_);
	}
}
void saved_game::write_carryover(config_writer& out) const
{
	if(!this->carryover_sides.empty())
	{
		out.write_child("carryover_sides", carryover_sides);	
	}
	if(!this->carryover_sides_start.empty())
	{
		out.write_child("carryover_sides_start", carryover_sides_start);	
	}
}


void saved_game::write_general_info(config_writer& out) const
{
	out.write(classification_.to_config());
	if (classification_.campaign_type == game_classification::MULTIPLAYER) {
		out.write_child("multiplayer", mp_settings_.to_config());
	}
}


void saved_game::expand_scenario()
{
	if(this->starting_pos_type_ == STARTINGPOS_NONE && !this->carryover_sides_start.empty())
	{
		resources::config_manager->load_game_config_for_game(this->classification());
		const config& game_config = resources::config_manager->game_config();
		const config& scenario = game_config.find_child(lexical_cast_default<std::string> (classification().campaign_type), "id", carryover_sides_start["next_scenario"]);
		if(scenario)
		{
			this->starting_pos_type_ = STARTINGPOS_SCENARIO;
			this->starting_pos_ = scenario;
		}
		else
		{
			this->starting_pos_type_ = STARTINGPOS_INVALID;
			this->starting_pos_ = config();
		}
	}
}

void saved_game::expand_carryover()
{
	expand_scenario();
	if(this->starting_pos_type_ == STARTINGPOS_SCENARIO && !this->carryover_sides_start.empty())
	{
		//TODO: Add events from modifications and modifications in side [mulitplayer][options][/options][/mulitplayer]
		
		carryover_info sides(carryover_sides_start);

		sides.transfer_to(get_starting_pos());
		BOOST_FOREACH(config& side_cfg, get_starting_pos().child_range("side")){
			sides.transfer_all_to(side_cfg);
		}

		carryover_sides = sides.to_config();
		carryover_sides_start = config();
	}
}

bool saved_game::valid()
{
	return this->starting_pos_type_ != STARTINGPOS_INVALID;
}

void saved_game::set_snapshot(const config& snapshot)
{
	this->starting_pos_type_ = STARTINGPOS_SNAPSHOT;
	this->starting_pos_ = snapshot;
}

void saved_game::set_scenario(const config& scenario)
{
	this->starting_pos_type_ = STARTINGPOS_SCENARIO;
	this->starting_pos_ = scenario;
}

void saved_game::remove_snapshot()
{
	this->starting_pos_type_ = STARTINGPOS_NONE;
	this->starting_pos_ = config();
}

config& saved_game::get_starting_pos()
{
	return starting_pos_;
}

void saved_game::remove_old_scenario()
{
	remove_snapshot();
	carryover_sides = config();
	replay_data = config();
	replay_start_ = config();
}

void saved_game::convert_to_start_save()
{
	assert(starting_pos_type_ == STARTINGPOS_SNAPSHOT);
	carryover_info sides(starting_pos_, true);
	sides.merge_old_carryover(carryover_info(carryover_sides));
	
	carryover_sides_start = sides.to_config();
	replay_data = config();
	replay_start_ = config();
	carryover_sides = config();
	remove_snapshot();
}
