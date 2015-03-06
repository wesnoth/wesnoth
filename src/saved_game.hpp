
#ifndef SAVED_GAME_HPP_INCLUDED
#define SAVED_GAME_HPP_INCLUDED

#include "config.hpp"
#include "game_classification.hpp"
#include "mp_game_settings.hpp"
#include "replay_recorder_base.hpp"

class config_writer;


class saved_game
{
	enum STARTING_POS_TYPE
	{
		/// There is no scenario stating pos data (start-of-scenario).
		STARTINGPOS_NONE,
		/// We have a [snapshot] (mid-game-savefile).
		STARTINGPOS_SNAPSHOT,
		/// We have a [scenario] (start-of-scenario) savefile.
		STARTINGPOS_SCENARIO,
		/// We failed to get a starting pos in expand_scenario.
		STARTINGPOS_INVALID
	};
public:
	saved_game();
	saved_game(const saved_game& state);
	//TODO: add or replace with saved_game(config&& cfg) constructor.
	explicit saved_game(const config& cfg);

	~saved_game(){}

	/// writes the config information into a stream (file)
	void write_config(config_writer& out) const;
	void write_general_info(config_writer& out) const;
	void write_carryover(config_writer& out) const;
	void write_starting_pos(config_writer& out) const;
	config to_config() const;
	game_classification& classification() { return classification_; }
	const game_classification& classification() const { return classification_; }

	/** Multiplayer parameters for this game */
	mp_game_settings& mp_settings() { return mp_settings_; }
	const mp_game_settings& mp_settings() const { return mp_settings_; }

	void set_carryover_sides_start(config carryover_sides_start);

	/// copies the content of a [scenario] with the correct id attribute from the game config into this object.
	/// reloads the game config from disk if necessary.
	void expand_scenario();
	/// merges [carryover_sides_start] into [scenario] and saves the rest into [carryover_sides]
	/// Removes [carryover_sides_start] afterwards
	void expand_carryover();
	/// adds [event]s from [era] and [modification] into this scenario
	/// does NOT expand [option]s because variables are persitent anyway to we don't need it
	/// should be called after expand_scenario() but before expand_carryover()
	void expand_mp_events();
	/// adds values of [option]s into [carryover_sides_start][variables] so that they are applied in the next level.
	/// Note that since [variabels] are persistent we only use this once at the beginning
	/// of a campaign but calling it multiple times is no harm eigher
	void expand_mp_options();
	/// takes care of generate_map=, generate_scenario=, map= attributes
	/// This should be called before expanding carryover or mp_events because this might completely replace starting_pos_.
	void expand_random_scenario();
	bool valid();
	/// @return the snapshot in the savefile (get_starting_pos)
	config& set_snapshot(config snapshot);
	void set_scenario(config scenario);
	void remove_snapshot();

	bool is_mid_game_save()
	{
		return starting_pos_type_ == STARTINGPOS_SNAPSHOT;
	}
	/// converts a normal savegame form the end of a scenaio to a start-of-scenario savefile for the next scenaio,
	/// The saved_game must contain a [snapshot] made during the linger mode of the last scenaio.
	void convert_to_start_save();
	/// sets the random seed if that didn't already happen.
	void set_random_seed();
	/// @return the starting pos for replays. Usualy this is [replay_start] but it can also be a [scenario] if no [replay_start] is present
	const config& get_replay_starting_pos();
	/// @return the id of the currently played scenario or the id of the next scenario if this is a between-scenaios-save (also called start-of-scenario-save).
	std::string get_scenario_id();
	/// @return the config from which the game will be started. (this is [scenario] or [snapshot] in the savefile)
	config& get_starting_pos();
	const config& get_starting_pos() const { return starting_pos_; }
	config& replay_start() { return replay_start_; }
	const config& replay_start() const { return replay_start_; }

	bool not_corrupt() const;
	/** sets classification().label to the correct value. */
	void update_label();

	void cancel_orders();
	/* removes network_ai and network controller types*/
	void unify_controllers();

	void set_default_save_id();
	replay_recorder_base& get_replay() { return replay_data_; }
	const replay_recorder_base& get_replay() const { return replay_data_; }
private:
	
	bool has_carryover_expanded_;
	/** 
		depends on has_carryover_expanded_:
		if true:  The carryover information for all sides from the previous scenario that aren't used in this scenario (to be carried over to the next scenario).
		if false: The carryover information for all sides from the previous scenario.
	*/
	config carryover_;
	/** snapshot made before the start event. To be used as a starting pos for replays */
	config replay_start_;
	/** some general information of the game that doesn't change during the game */
	game_classification classification_;
	mp_game_settings mp_settings_;

	STARTING_POS_TYPE starting_pos_type_;
	/**
		The starting pos where the (non replay) game will be started from.
		This can eigher be a [scenario] for a fresh game or a [snapshot] if this is a reloaded game
	*/
	config starting_pos_;

	replay_recorder_base replay_data_;
};


#endif
