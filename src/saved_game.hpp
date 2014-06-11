
#ifndef SAVED_GAME_HPP_INCLUDED
#define SAVED_GAME_HPP_INCLUDED

#include "config.hpp"
#include "gamestatus.hpp" //game_classification
class config_writer;


class saved_game
{
	enum STARTING_POS_TYPE
	{
		STARTINGPOS_NONE,
		STARTINGPOS_SNAPSHOT,
		STARTINGPOS_SCENARIO,
		STARTINGPOS_INVALID
	};
public:
	saved_game();
	saved_game(const saved_game& state);
	explicit saved_game(const config& cfg);

	~saved_game(){}
	saved_game& operator=(const saved_game& state);
	
	//write the config information into a stream (file)
	void write_config(config_writer& out) const;
	void write_general_info(config_writer& out) const;
	void write_carryover(config_writer& out) const;
	void write_starting_pos(config_writer& out) const;

	game_classification& classification() { return classification_; }
	const game_classification& classification() const { return classification_; }

	/** Multiplayer parameters for this game */
	mp_game_settings& mp_settings() { return mp_settings_; }
	const mp_game_settings& mp_settings() const { return mp_settings_; }

	void expand_scenario();
	void expand_carryover();
	bool valid();
	void set_snapshot(const config& snapshot);
	void set_scenario(const config& scenario);
	void remove_snapshot();
	config& get_starting_pos();
	config& replay_start() { return replay_start_; }

	bool is_mid_game_save()
	{
		return starting_pos_type_ == STARTINGPOS_SNAPSHOT;
	}
	/**
	 * If the game is saved mid-level, we have a series of replay steps
	 * to take the game up to the position it was saved at.
	 */
	config replay_data;

	/** The carryover information for all sides*/
	config carryover_sides;

	/** The carryover information for all sides as it was before the scenario started*/
	config carryover_sides_start;

private:
	/** First turn snapshot for replays, contains starting position */
	config replay_start_;

	game_classification classification_;
	mp_game_settings mp_settings_;

	
	/**
	 * Snapshot of the game's current contents.
	 *
	 * i.e. unless the player selects to view a replay, the game's settings are
	 * read in from this object.
	 */
	STARTING_POS_TYPE starting_pos_type_;
	config starting_pos_;
};


#endif
