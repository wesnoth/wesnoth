#ifndef MULTIPLAYER_LOBBY_INCLUDED
#define MULTIPLAYER_LOBBY_INCLUDED

#include "config.hpp"
#include "display.hpp"

namespace lobby {

enum RESULT { QUIT, CREATE, JOIN };

RESULT enter(display& disp, config& data);

}

#endif
