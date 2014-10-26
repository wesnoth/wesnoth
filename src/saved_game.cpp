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
#include "carryover.hpp"
#include "cursor.hpp"
#include "log.hpp"
#include "resources.hpp"
#include "game_config_manager.hpp"
#include "generators/map_create.hpp"
#include "statistics.hpp"
#include "serialization/binary_or_text.hpp"
#include "util.hpp"

#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/foreach.hpp>
#include <cassert>

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)
#define WRN_NG LOG_STREAM(warn, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)
#define DBG_NG LOG_STREAM(debug, log_engine)

saved_game::saved_game()
	: replay_data()
	, has_carryover_expanded_(false)
	, carryover_(carryover_info().to_config())
	, replay_start_()
	, classification_()
	, mp_settings_()
	, starting_pos_type_(STARTINGPOS_NONE)
	, starting_pos_()
{

}

saved_game::saved_game(const config& cfg)
	: replay_data()
	, has_carryover_expanded_(false)
	, carryover_()
	, replay_start_()
	, classification_(cfg)
	, mp_settings_(cfg)
	, starting_pos_type_(STARTINGPOS_NONE)
	, starting_pos_()

{
	log_scope("read_game");

	if(const config & caryover_sides = cfg.child("carryover_sides"))
	{
		carryover_ = caryover_sides;
		has_carryover_expanded_ = true;
	}
	else
	{
		carryover_ = cfg.child_or_empty("carryover_sides_start");
	}

	
	//Serversided replays can contain multiple [replay]
	replay_start_ = cfg.child_or_empty("replay_start");
	replay_data = config(); //cfg.child_or_empty("replay");
	BOOST_FOREACH(const config& replay, cfg.child_range("replay"))
	{
		replay_data.append_children(replay);
	}

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

	LOG_NG << "scenario: '" << carryover_["next_scenario"].str() << "'\n";

	if (const config &stats = cfg.child("statistics")) {
		statistics::fresh_stats();
		statistics::read_stats(stats);
	}

}

saved_game::saved_game(const saved_game& state)
	: replay_data(state.replay_data)
	, has_carryover_expanded_(state.has_carryover_expanded_)
	, carryover_(state.carryover_)
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

void saved_game::set_carryover_sides_start(config carryover_sides_start)
{
	carryover_.swap(carryover_sides_start);
	has_carryover_expanded_ = false;
}

void saved_game::set_random_seed()
{
	if(has_carryover_expanded_ || !carryover_["random_seed"].empty())
	{
		return;
	}
	carryover_["random_seed"] = rand();
	carryover_["random_calls"] = 0;
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
	assert(not_corrupt());
	out.write_child(has_carryover_expanded_ ? "carryover_sides" : "carryover_sides_start", carryover_);
}


void saved_game::write_general_info(config_writer& out) const
{
	out.write(classification_.to_config());
	if (classification_.campaign_type == game_classification::MULTIPLAYER
			|| classification_.campaign_type == game_classification::SCENARIO) {
		out.write_child("multiplayer", mp_settings_.to_config());
	}
}


void saved_game::expand_scenario()
{
	if(this->starting_pos_type_ == STARTINGPOS_NONE && !has_carryover_expanded_)
	{
		resources::config_manager->load_game_config_for_game(this->classification());
		const config& game_config = resources::config_manager->game_config();
		const config& scenario = game_config.find_child(lexical_cast_default<std::string>
				(classification().campaign_type == game_classification::SCENARIO ?
				 game_classification::MULTIPLAYER : classification().campaign_type),
				"id", carryover_["next_scenario"]);
		if(scenario)
		{
			this->starting_pos_type_ = STARTINGPOS_SCENARIO;
			this->starting_pos_ = scenario;
			// A hash has to be generated using an unmodified scenario data.
			mp_settings_.hash = scenario.hash();

			update_label();

			//Set this default value immideately after reading the scenario is importent because otherwise
			//we might endup settings this value to the multiplayer players name, which would break carryover.
			//(doing this in at config loading in game_config would be ok too i think.)
			BOOST_FOREACH(config& side, starting_pos_.child_range("side"))
			{
				if(side["save_id"].str() == "")
				{
					side["save_id"] = side["id"];
				}
			}
		}
		else
		{
			this->starting_pos_type_ = STARTINGPOS_INVALID;
			this->starting_pos_ = config();
		}
	}
}

//helper objects for saved_game::expand_mp_events()
struct modevents_entry
{
	modevents_entry(const std::string& _type, const std::string& _id) : type(_type), id(_id) {}
	std::string type;
	std::string id;
};
struct modevents_entry_for
{
	//this typedef is used by boost.
    typedef modevents_entry result_type;
	modevents_entry_for(const std::string& type ) : type_(type) {}
	modevents_entry operator()(const std::string& id) const
	{
		return modevents_entry(type_, id);
	}
private:
	std::string type_;
};

void saved_game::expand_mp_events()
{
	expand_scenario();
	if(this->starting_pos_type_ == STARTINGPOS_SCENARIO && !this->starting_pos_["has_mod_events"].to_bool(false))
	{
		std::vector<modevents_entry> mods;

		boost::copy( mp_settings_.active_mods
			| boost::adaptors::transformed(modevents_entry_for("modification"))
			, std::back_inserter(mods) );
		if(mp_settings_.mp_era != "") //We don't want the error message below if there is no era (= if this is a sp game)
		{ mods.push_back(modevents_entry("era", mp_settings_.mp_era)); }

		BOOST_FOREACH(modevents_entry& mod, mods)
		{
			if(const config& cfg = resources::config_manager->
				game_config().find_child(mod.type, "id", mod.id))
			{
				BOOST_FOREACH(const config& modevent, cfg.child_range("event"))
				{
					this->starting_pos_.add_child("event", modevent);
				}
				BOOST_FOREACH(const config& modlua, cfg.child_range("lua"))
				{
					this->starting_pos_.add_child("lua", modlua);
				}
			}
			else
			{
				//TODO: A user message instead?
				ERR_NG << "Couldn't find [" << mod.type<< "] with id=" << mod.id <<std::endl;
			}
		}

		this->starting_pos_["has_mod_events"] = true;
	}
}

void saved_game::expand_mp_options()
{
	if(starting_pos_type_ == STARTINGPOS_SCENARIO && !has_carryover_expanded_)
	{
		std::vector<modevents_entry> mods;

		boost::copy( mp_settings_.active_mods
			| boost::adaptors::transformed(modevents_entry_for("modification"))
			, std::back_inserter(mods) );
		mods.push_back(modevents_entry("era", mp_settings_.mp_era));
		mods.push_back(modevents_entry("multiplayer", get_scenario_id()));

		config& variables = carryover_.child_or_add("variables");
		BOOST_FOREACH(modevents_entry& mod, mods)
		{
			if(const config& cfg = this->mp_settings().options.find_child(mod.type, "id", mod.id))
			{
				BOOST_FOREACH(const config& option, cfg.child_range("option"))
				{
					variables[option["id"]] = option["value"];
				}
			}
			else
			{
				LOG_NG << "Couldn't find [" << mod.type<< "] with id=" << mod.id << " for [option]s" << std::endl;
			}
		}
	}
}

void saved_game::expand_random_scenario()
{
	expand_scenario();
	if(this->starting_pos_type_ == STARTINGPOS_SCENARIO)
	{
		// If the entire scenario should be randomly generated
		if(!starting_pos_["scenario_generation"].empty())
		{
			LOG_NG << "randomly generating scenario...\n";
			const cursor::setter cursor_setter(cursor::WAIT);

			config scenario_new = random_generate_scenario(starting_pos_["scenario_generation"],
				starting_pos_.child("generator"));
			//Preserve "story" form the scenario toplevel.
			BOOST_FOREACH(config& story, starting_pos_.child_range("story"))
			{
				scenario_new.add_child("story", story);
			}
			scenario_new["id"] = starting_pos_["id"]; 
			starting_pos_ = scenario_new;
			update_label();
		}
		//it looks like we support a map= where map=filename equals more or less map_data={filename}
		if(starting_pos_["map_data"].empty() && !starting_pos_["map"].empty()) {
			starting_pos_["map_data"] = filesystem::read_map(starting_pos_["map"]);
		}
		// If the map should be randomly generated
		// We dont want that we accidently to this twice so we check for starting_pos_["map_data"].empty()
		if(starting_pos_["map_data"].empty() && !starting_pos_["map_generation"].empty()) {
			LOG_NG << "randomly generating map...\n";
			const cursor::setter cursor_setter(cursor::WAIT);

			starting_pos_["map_data"] = random_generate_map(
				starting_pos_["map_generation"], starting_pos_.child("generator"));
		}
	}
}

void saved_game::expand_carryover()
{
	expand_scenario();
	if(this->starting_pos_type_ == STARTINGPOS_SCENARIO && !has_carryover_expanded_)
	{
		carryover_info sides(carryover_);

		sides.transfer_to(get_starting_pos());
		BOOST_FOREACH(config& side_cfg, get_starting_pos().child_range("side")){
			sides.transfer_all_to(side_cfg);
		}

		carryover_ = sides.to_config();
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
	has_carryover_expanded_ = false;
	update_label();
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


const config& saved_game::get_replay_starting_pos()
{
	if(!replay_start_.empty())
	{
		return replay_start_;
	}
	if(!has_carryover_expanded_)
	{
		//Try to load the scenario form game config or from [scenario] if there is no [replay_start]
		expand_scenario();
		expand_carryover();
	}
	if(starting_pos_type_ == STARTINGPOS_SCENARIO)
	{
		return starting_pos_;
	}
	return this->replay_start_.child("some_non_existet_invalid");
}

void saved_game::convert_to_start_save()
{
	assert(starting_pos_type_ == STARTINGPOS_SNAPSHOT);
	carryover_info sides(starting_pos_, true);
	sides.merge_old_carryover(carryover_info(carryover_));
	sides.rng().rotate_random();
	carryover_ = sides.to_config();
	has_carryover_expanded_ = false;
	replay_data = config();
	replay_start_ = config();
	remove_snapshot();
}

config saved_game::to_config() const
{
	//TODO: remove this code dublication with write_... functions.
	config r = classification_.to_config();
	if(!this->replay_start_.empty())
	{
		r.add_child("replay_start", replay_start_);
	}
	if(!this->replay_data.empty())
	{
		r.add_child("replay", replay_data);
	}
	if(starting_pos_type_ == STARTINGPOS_SNAPSHOT)
	{
		r.add_child("snapshot", starting_pos_);
	}
	else if(starting_pos_type_ == STARTINGPOS_SCENARIO)
	{
		r.add_child("scenario", starting_pos_);
	}
	r.add_child(has_carryover_expanded_ ? "carryover_sides" : "carryover_sides_start" , carryover_);
	if (classification_.campaign_type == game_classification::MULTIPLAYER
		|| classification_.campaign_type == game_classification::SCENARIO) {
		r.add_child("multiplayer", mp_settings_.to_config());
	}
	return r;
}

std::string saved_game::get_scenario_id()
{
	std::string scenario_id;
	if(this->starting_pos_type_ == STARTINGPOS_SNAPSHOT
		|| this->starting_pos_type_ == STARTINGPOS_SCENARIO)
	{
		scenario_id = starting_pos_["id"].str();
	}
	else if(!has_carryover_expanded_)
	{
		scenario_id = carryover_["next_scenario"].str();
	}
	else
	{
		assert(!"cannot figure out scenario_id");
		throw "assertion ingnored";
	}
	return scenario_id == "null" ? "" : scenario_id;
}

bool saved_game::not_corrupt() const
{
	return true;
}

void saved_game::update_label()
{
	if (classification().abbrev.empty())
		classification().label = starting_pos_["name"].str();
	else {
		classification().label = classification().abbrev + "-" + starting_pos_["name"];
	}
}

void saved_game::cancel_orders()
{
	BOOST_FOREACH(config &side, this->starting_pos_.child_range("side"))
	{
		// for humans "goto_x/y" is used for multi-turn-moves
		// for the ai "goto_x/y" is a way for wml to order the ai to move a unit to a certain place.
		// we want to cancel human order but not to break wml.
		if (side["controller"] != "human" && side["controller"] != "network") continue;
		BOOST_FOREACH(config &unit, side.child_range("unit"))
		{
			unit["goto_x"] = -999;
			unit["goto_y"] = -999;
		}
	}
}

void saved_game::unify_controllers()
{
	BOOST_FOREACH(config &side, this->starting_pos_.child_range("side"))
	{
		if (side["controller"] == "network")
			side["controller"] = "human";
		if (side["controller"] == "network_ai")
			side["controller"] = "ai";
	}
}
