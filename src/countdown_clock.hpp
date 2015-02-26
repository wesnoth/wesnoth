#pragma once
#include "events.hpp"
#include <SDL_timer.h>

class team;
struct mp_game_settings;
class countdown_clock : public events::pump_monitor
{
public:
	countdown_clock(team& team);
	~countdown_clock();
	/// @returns ticks passed since last update
	/// @param new_timestamp latest result of SDL_GetTicks()
	int update_timestamp(int new_timestamp);
	/// @param new_timestamp latest result of SDL_GetTicks()
	void update_team(int new_timestamp);
	void process(events::pump_info &info);
	///	@return whether there is time left
	/// @param new_timestamp latest result of SDL_GetTicks()
	bool update(int new_timestamp = SDL_GetTicks());
	void maybe_play_sound();
private:
	team& team_;
	int last_timestamp_;
	bool playing_sound_;
};

