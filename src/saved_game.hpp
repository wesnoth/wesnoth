
#ifndef SAVED_GAME_HPP_INCLUDED
#define SAVED_GAME_HPP_INCLUDED

#include "config.hpp"
#include "gamestatus.hpp" //game_classification

class config_writer;


class saved_game
{
public:
	saved_game();
	saved_game(const saved_game& state);
	explicit saved_game(const config& cfg);

	~saved_game(){}
	saved_game& operator=(const saved_game& state);

	//write the config information into a stream (file)
	void write_config(config_writer& out) const;

	game_classification& classification() { return classification_; }
	const game_classification& classification() const { return classification_; }

	/** Multiplayer parameters for this game */
	mp_game_settings& mp_settings() { return mp_settings_; }
	const mp_game_settings& mp_settings() const { return mp_settings_; }

	config& replay_start() { return replay_start_; }

	/**
	 * If the game is saved mid-level, we have a series of replay steps
	 * to take the game up to the position it was saved at.
	 */
	config replay_data;

	/**
	 * Snapshot of the game's current contents.
	 *
	 * i.e. unless the player selects to view a replay, the game's settings are
	 * read in from this object.
	 */
	config snapshot;

	/** The carryover information for all sides*/
	config carryover_sides;

	/** The carryover information for all sides as it was before the scenario started*/
	config carryover_sides_start;

private:
	/** First turn snapshot for replays, contains starting position */
	config replay_start_;

	game_classification classification_;
	mp_game_settings mp_settings_;
};


#endif
