#ifndef MULTIPLAYER_CLIENT_HPP_INCLUDED
#define MULTIPLAYER_CLIENT_HPP_INCLUDED

#include "config.hpp"
#include "display.hpp"
#include "gamestatus.hpp"
#include "network.hpp"
#include "unit_types.hpp"

//function to play a game as a client, joining either another player who is
//hosting a game, or connecting to a server.
void play_multiplayer_client(display& disp, game_data& units_data, config& cfg,
                             game_state& state, std::string& host);


#endif
