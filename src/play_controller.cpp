#include "play_controller.hpp"
#include "replay.hpp"

play_controller::play_controller(const config& level, game_state& state_of_game, 
								 int ticks, int num_turns, const config& game_config) : 
	level_(level), ticks_(ticks),
	gamestate_(state_of_game), status_(level, num_turns), statistics_context_(level_["name"]),
	game_config_(game_config), map_(game_config, level["map_data"])
{
	init();
}

void play_controller::init(){
	//if the recorder has no event, adds an "game start" event to the
	//recorder, whose only goal is to initialize the RNG
	if(recorder.empty()) {
		recorder.add_start();
	} else {
		recorder.pre_replay();
	}
	recorder.set_skip(false);
}