#ifndef MULTIPLAYER_LOBBY_INCLUDED
#define MULTIPLAYER_LOBBY_INCLUDED

#include "config.hpp"
#include "display.hpp"

//this module controls the multiplayer lobby. A section on the server which
//allows players to chat, create games, and join games.
namespace lobby {

enum RESULT { QUIT, CREATE, JOIN };

//function which controls the lobby, and will result in the player creating
//a game, joining a game, or quitting the lobby.
RESULT enter(display& disp, config& data, const config& terrain_data);

}

#endif
